#include <ovito/particles/Particles.h>
#include <ovito/particles/objects/Particles.h>
#include <ovito/particles/objects/ParticleType.h>
#include <ovito/stdobj/simcell/SimulationCell.h>

#include "TprImporter.h"

#include <tprparser/Reader.h>

namespace Ovito {
IMPLEMENT_OVITO_CLASS(TprImporter);

bool TprImporter::OOMetaClass::checkFileFormat(const FileHandle& file) const 
{
    return file.sourceUrl().fileName().endsWith(QStringLiteral(".tpr"), Qt::CaseInsensitive);
}


void TprImporter::FrameLoader::loadFile()
{
    setProgressText(tr("Reading Gromacs Tpr file: %1").arg(fileHandle().toString()));
	try
	{
		auto tpr = std::make_unique<TprReader>(
			QFile::encodeName(QDir::toNativeSeparators(fileHandle().localFilePath())).constData()
		);

		constexpr float factor = 10.0f; // nm to angstrom

		// allocated
		const auto& pos = tpr->get_xvf("x");
		size_t natoms = pos.size() / 3u;
		size_t nbonds, nangles, ndihedrals, nimpropers;
		nbonds = nangles = ndihedrals = nimpropers = 0;
		setProgressMaximum(natoms);
		setParticleCount(natoms);


		// set box
		std::vector<float> box{ tpr->get_xvf("box") };
		// box nm to angstrom
		std::transform(box.begin(), box.end(), box.begin(), [](float &val) {return val * factor; });
		AffineTransformation cell = AffineTransformation::Identity();
		// need check
		cell(0, 0) = box[0]; cell(0, 1) = box[3]; cell(0, 2) = box[6];
		cell(1, 0) = box[1]; cell(1, 1) = box[4]; cell(1, 2) = box[7];
		cell(2, 0) = box[2]; cell(2, 1) = box[5]; cell(2, 2) = box[8];
		simulationCell()->setCellMatrix(cell);

		// Create standard particle properties.
		Property *positions = particles()->createProperty(DataBuffer::Initialized, Particles::PositionProperty);
		Property *types = particles()->createProperty(DataBuffer::Initialized, Particles::TypeProperty);
		Property *atomnames = particles()->createProperty(QStringLiteral("Atom Name"), Property::Int32);
		// gmx forcefield atom type
		Property *gmxtypes = particles()->createProperty(QStringLiteral("Forcefield Type"), Property::Int32);
		Property *residuetypes = particles()->createProperty(QStringLiteral("Residue Type"), Property::Int32);
		Property *resids = particles()->createProperty(QStringLiteral("Residue Identifier"), Property::Int64);
		// particle serial, 1-based
		BufferWriteAccess<int64_t, access_mode::read_write> identifierProperty = particles()->createProperty(DataBuffer::Initialized, Particles::IdentifierProperty);
		// particle velocities
		BufferWriteAccess<Vector3, access_mode::discard_write> accVel;
		// other access property
		BufferWriteAccess<Point3, access_mode::read_write>     accPositions(positions);
		BufferWriteAccess<int32_t, access_mode::discard_write> accTypes(types);
		BufferWriteAccess<int32_t, access_mode::discard_write> accgmxtypes(gmxtypes);
		BufferWriteAccess<int32_t, access_mode::discard_write> accAtomnames(atomnames);
		BufferWriteAccess<int32_t, access_mode::discard_write> accResiduetypes(residuetypes);
		BufferWriteAccess<int64_t, access_mode::discard_write> accResids(resids);

		// user interface titles
		atomnames->setTitle(tr("Atom names"));
		residuetypes->setTitle(tr("Residue types"));
		gmxtypes->setTitle(tr("Forcefield Types"));

		// if has velocity
		bool hasVel = true;
		const std::vector<float>* vel = nullptr;
		try {
			vel = &(tpr->get_xvf("v"));
		}
		catch (...) {
			hasVel = false;
		}
		if (hasVel) {
			accVel = particles()->createProperty(Particles::VelocityProperty);
		}

		// from tpr
		const auto& _typenames = tpr->get_name("type");
		const auto& _resnames = tpr->get_name("res");
		const auto& _atomnames = tpr->get_name("atom");
		const auto& _resids = tpr->get_ivector("resid");

		for (size_t i = 0; i < natoms; i++) 
		{
			// serial, 1-based
			identifierProperty[i] = i + 1;

			// need nm to angstrom
			accPositions[i][0] = pos[3L * i] * factor;
			accPositions[i][1] = pos[3L * i + 1] * factor;
			accPositions[i][2] = pos[3L * i + 2] * factor;

			// nm/ps -> angstrom/ps
			if (hasVel) {
				accVel[i][0] = vel->at(3L * i) * factor;
				accVel[i][1] = vel->at(3L * i + 1) * factor;
				accVel[i][2] = vel->at(3L * i + 2) * factor;
			}

			// Particle Types name use atom name is good for auto get vdw radius
			accTypes[i] = addNamedType(Particles::OOClass(), types, QLatin1String(_atomnames[i]))->numericId();
			accgmxtypes[i] = addNamedType(Particles::OOClass(), gmxtypes, QLatin1String(_typenames[i]))->numericId();
			accResids[i] = _resids[i];
			accResiduetypes[i] = addNamedType(Particles::OOClass(), residuetypes, QLatin1String(_resnames[i]))->numericId();
			accAtomnames[i] = addNamedType(Particles::OOClass(), atomnames, QLatin1String(_atomnames[i]))->numericId();
		}
		
		// reorder by name
		types->sortElementTypesByName();
		residuetypes->sortElementTypesByName();
		atomnames->sortElementTypesByName();
		gmxtypes->sortElementTypesByName();

		// atomic charge and mass
		{
			// atomic charge, mass
			try
			{
				const auto& _charges = tpr->get_xvf("q");
				const auto& _masses = tpr->get_xvf("m");
				BufferWriteAccess<FloatType, access_mode::discard_write> accCharge = particles()->createProperty(DataBuffer::Initialized, Particles::ChargeProperty);
				BufferWriteAccess<FloatType, access_mode::discard_write> accMass = particles()->createProperty(DataBuffer::Initialized, Particles::MassProperty);
				boost::copy(_charges, accCharge.begin());
				boost::copy(_masses, accMass.begin());
			}
			catch (...)
			{
				// No mass or charges?
			}
		}


		// bonded: bonds, angle, dihedrals, should remove repetition!
		{
			try
			{
				const auto& o = tpr->get_bonded("bonds");
				std::set<std::pair<int, int>> _bonds;
				for (const auto& bond : o) {
					_bonds.emplace(bond.a - 1, bond.b - 1); // Ovito atomid 0-based
				}
				nbonds = _bonds.size();
				setProgressMaximum(nbonds);
				setBondCount(nbonds);

				BufferWriteAccess<ParticleIndexPair, access_mode::discard_write> btop = bonds()->createProperty(Bonds::TopologyProperty);
				auto ptr = btop.begin();
				for (const auto& bond : _bonds) {
					(*ptr)[0] = bond.first;
					(*ptr)[1] = bond.second;
					++ptr;
				}
			}
			catch (...)
			{
				// No bonds
			}
		}

		{
			try
			{
				const auto& o = tpr->get_bonded("angles");
				std::set<std::tuple<int, int, int>> _angles;
				for (const auto& angle : o) {
					_angles.emplace(angle.a - 1, angle.b - 1, angle.c - 1);
				}
				nangles = _angles.size();
				setProgressMaximum(nangles);
				setAngleCount(nangles);

				BufferWriteAccess<ParticleIndexTriplet, access_mode::discard_write> atop = angles()->createProperty(Angles::TopologyProperty);
				auto ptr = atop.begin();
				for (const auto& angle : _angles) {
					(*ptr)[0] = std::get<0>(angle);
					(*ptr)[1] = std::get<1>(angle);
					(*ptr)[2] = std::get<2>(angle);
					++ptr;
				}
			}
			catch (...)
			{
				// No angles
			}
		}

		{
			const std::vector<const char *> dihtypes = { "dihedrals", "impropers" };
			for (size_t i = 0; i < dihtypes.size(); i++)
			{
				try
				{
					const auto& o = tpr->get_bonded(dihtypes[i]);
					std::set<std::tuple<int, int, int, int>> _dihedrals;
					for (const auto& dih : o) {
						_dihedrals.emplace(dih.a - 1, dih.b - 1, dih.c - 1, dih.d - 1);
					}
					size_t tempcount = _dihedrals.size();
					setProgressMaximum(tempcount);

					BufferWriteAccess<ParticleIndexQuadruplet, access_mode::discard_write> dtop;
					// propers dihedrals
					if (i == 0) {
						ndihedrals = tempcount;
						setDihedralCount(tempcount);
						dtop = dihedrals()->createProperty(Dihedrals::TopologyProperty);
					}
					// impropers
					else {
						nimpropers = tempcount;
						setImproperCount(tempcount);
						dtop = impropers()->createProperty(Impropers::TopologyProperty);
					}

					// store data
					auto ptr = dtop.begin();
					for (const auto& dih : _dihedrals) {
						(*ptr)[0] = std::get<0>(dih);
						(*ptr)[1] = std::get<1>(dih);
						(*ptr)[2] = std::get<2>(dih);
						(*ptr)[3] = std::get<3>(dih);
						++ptr;
					}
				}
				catch (...)
				{
					// No dihedrals or imporpers
				}
			}
		}		

		// show info to Status
		state().setStatus(
			tr("%1 atoms, %2 bonds, %3 angles, %4 dihedrals, %5 impropers").
			arg(natoms).arg(nbonds).arg(nangles).arg(ndihedrals).arg(nimpropers)
		);

		// Call base implementation to finalize the loaded particle data.
		ParticleImporter::FrameLoader::loadFile();
	}
	catch (const std::exception&e)
	{
		throw Exception(tr("TprParser error: %1").arg(e.what()));
	}
}

} // end of namespace Ovito
