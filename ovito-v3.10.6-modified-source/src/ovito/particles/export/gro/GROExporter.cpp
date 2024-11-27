#include <ovito/particles/Particles.h>
#include <ovito/particles/objects/Particles.h>
#include <ovito/stdobj/simcell/SimulationCell.h>
#include "GROExporter.h"


namespace Ovito {

IMPLEMENT_OVITO_CLASS(GROExporter);

bool GROExporter::exportData(const PipelineFlowState& state, int frameNumber, const QString& filePath, MainThreadOperation& operation)
{
    // get particle positions
    const Particles* particles = state.expectObject<Particles>();
    particles->verifyIntegrity();

    size_t natoms = particles->elementCount();
    if(natoms < 1) {
        throw Exception(tr("No atoms found. Cannot write GRO file."));
    }
    textStream() << "Written by Ovito\n";
    textStream() << natoms << "\n";

    // simulation cell.
    Point3 origin = Point3::Origin();
    const SimulationCell* simulationCell = state.getObject<SimulationCell>();
    if(simulationCell) {
        origin = simulationCell->cellOrigin();
    }

    // get atoms property
    auto typeprop = particles->getProperty(Particles::TypeProperty);
    auto restypeprop = particles->getProperty("Residue Type");
    auto atomnameprop = particles->getProperty("Atom Name");
    auto residprop = particles->getProperty("Residue Identifier");
    // check properties
    if(!typeprop) {
        throw Exception(tr("Can not get TypeProperty"));
    }

    BufferReadAccess<Point3> positions = particles->expectProperty(Particles::PositionProperty);
    BufferReadAccess<Point3> velocity = particles->getProperty(Particles::VelocityProperty);
    BufferReadAccess<int32_t> particleTypeArray(typeprop);

    BufferReadAccess<int32_t> resnames(restypeprop);
    BufferReadAccess<int32_t> atomnames(atomnameprop);
    BufferReadAccess<int64_t> resids(residprop);

    operation.setProgressMaximum(positions.size());
    constexpr auto Factor = FloatType(0.1); // Angstrom to nm factor
    for(size_t i = 0; i < positions.size(); i++) {
        const auto pos = (positions[i] - origin) * Factor;

        // set up atomname
        QString name;
        // first try get atom name from Atom Name property
        const ElementType* s = atomnames ? atomnameprop->elementType(atomnames[i]) : nullptr;
        if(s && !s->name().isEmpty()) {
            name = s->name();
        }
        // second try from Type peoperty
        if(name.isEmpty()) {
            const ElementType* type = particleTypeArray ? typeprop->elementType(particleTypeArray[i]) : nullptr;
            if(type && !type->name().isEmpty()) {
                name = type->name();
            }
            else if(particleTypeArray) {
                name = tr("%1").arg(particleTypeArray[i]);
            }
            else {
                throw Exception(tr("Can not get atom name, it is needed for GRO format."));
            }
        }


        // set up resid
        int64_t resid = 1;
        if(resids) {
            resid = resids[i];
        }

        // set up resname
        QString resname = "UNK";
        const ElementType* str = resnames ? restypeprop->elementType(resnames[i]) : nullptr;
        if(str && !str->name().isEmpty()) {
            resname = str->name();
        }
        else if(resnames) {
            resname = tr("%1").arg(resnames[i]);
        }

        textStream() << tr("%1%2%3%4%5%6%7")
                        .arg(resid % 100000, 5)
                        .arg(resname, -5)
                        .arg(name, 5)
                        .arg((i + 1) % 100000, 5)
                        .arg(pos.x(), 8, 'f', 3)
                        .arg(pos.y(), 8, 'f', 3)
                        .arg(pos.z(), 8, 'f', 3);
        if(velocity) {
            // the velocity unit not need convert
            const auto &vel = velocity[i];
            textStream() << tr("%1%2%3\n")
                        .arg(vel.x(), 8, 'f', 4)
                        .arg(vel.y(), 8, 'f', 4)
                        .arg(vel.z(), 8, 'f', 4);
        }
        else {
            textStream() << "\n";
        }

        if(!operation.setProgressValueIntermittent(i)) {
            return false;    
        }
    }

    // write box
    if(simulationCell) {
        const AffineTransformation cell = simulationCell->cellMatrix() * Factor;
        textStream() 
            << cell(0, 0) << " " << cell(1, 1) << " " << cell(2, 2) << " " 
            << cell(1, 0) << " " << cell(2, 0) << " " << cell(0, 1) << " " 
            << cell(2, 1) << " " << cell(0, 2) << " " << cell(1, 2) << "\n";
    }
    else {
        textStream() << "0.0 0.0 0.0\n";
    }

    return !operation.isCanceled();
}

}  // namespace Ovito
