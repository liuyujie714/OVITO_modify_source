////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2023 OVITO GmbH, Germany
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify it either under the
//  terms of the GNU General Public License version 3 as published by the Free Software
//  Foundation (the "GPL") or, at your option, under the terms of the MIT License.
//  If you do not alter this notice, a recipient may use your version of this
//  file under either the GPL or the MIT License.
//
//  You should have received a copy of the GPL along with this program in a
//  file LICENSE.GPL.txt.  You should have received a copy of the MIT License along
//  with this program in a file LICENSE.MIT.txt
//
//  This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND,
//  either express or implied. See the GPL or the MIT License for the specific language
//  governing rights and limitations.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <ovito/particles/Particles.h>
#include <ovito/particles/objects/Particles.h>
#include <ovito/particles/objects/ParticleType.h>
#include <ovito/stdobj/simcell/SimulationCell.h>
#include "TRRImporter.h"

#include <xdrfile/xdrfile.h>
#include <xdrfile/xdrfile_trr.h>

namespace Ovito {

IMPLEMENT_OVITO_CLASS(TRRImporter);

class TRRFile
{
public:

    struct Frame {
        int step;
        float time;
        float lambda;
        int has_prop = 0; // HASX/HASV/HASF
        Matrix_3<float> cell;
        std::vector<Point_3<float>> xyz;
        std::vector<Point_3<float>> vel; // velocity
        std::vector<Point_3<float>> force; // force
    };

    ~TRRFile() {
        close();
    }

    void open(const char* filename) {
        close();
        int returnCode = read_trr_natoms((char *)filename, &_numAtoms);
        if(returnCode != exdrOK || _numAtoms <= 0)
            throw Exception(TRRImporter::tr("Error opening TRR file (error code %1).").arg(returnCode));
        _file = xdrfile_open(filename, "r");
        if(!_file)
            throw Exception(TRRImporter::tr("Error opening TRR file."));
        _eof = false;
    }

    void close() {
        if(_file) {
            if(xdrfile_close(_file) != exdrOK)
                qWarning() << "TRRImporter: Failure reported by xdrfile_close()";
            _file = nullptr;
        }
    }

    Frame read() {
        OVITO_ASSERT(_file);
        OVITO_ASSERT(!_eof);
        Frame frame;
        frame.xyz.resize(_numAtoms);
        frame.vel.resize(_numAtoms);
        frame.force.resize(_numAtoms);
        int returnCode = read_trr(_file, _numAtoms, &frame.step, &frame.time, &frame.lambda,
            reinterpret_cast<matrix&>(frame.cell), 
            reinterpret_cast<rvec*>(frame.xyz.data()), 
            reinterpret_cast<rvec*>(frame.vel.data()), 
            reinterpret_cast<rvec*>(frame.force.data()), 
            &frame.has_prop);
        // modified do_trnheader: add exdrENDOFFILE
        if(returnCode != exdrOK && returnCode != exdrENDOFFILE)
            throw Exception(TRRImporter::tr("Error reading TRR file (code %1).").arg(returnCode));
        if(returnCode == exdrENDOFFILE)
            _eof = true;
        return frame;
    }

    void seek(qint64 offset) {
        int returnCode = xdr_seek(_file, offset, SEEK_SET);
        if(returnCode != exdrOK)
            throw Exception(TRRImporter::tr("Error seeking in TRR file (code %1).").arg(returnCode));
    }

    qint64 byteOffset() const {
        OVITO_ASSERT(_file);
        return xdr_tell(_file);
    }

    bool eof() const { return _eof; }

private:

    XDRFILE* _file = nullptr;
    int _numAtoms = 0;
    bool _eof = false;
};

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool TRRImporter::OOMetaClass::checkFileFormat(const FileHandle& file) const
{
    return file.sourceUrl().fileName().endsWith(QStringLiteral(".trr"), Qt::CaseInsensitive);
}

/******************************************************************************
* Scans the data file and builds a list of source frames.
******************************************************************************/
void TRRImporter::FrameFinder::discoverFramesInFile(QVector<FileSourceImporter::Frame>& frames)
{
    setProgressText(tr("Scanning file %1").arg(fileHandle().toString()));
    setProgressMaximum(QFileInfo(fileHandle().localFilePath()).size());

    // Open TRR file for reading.
    TRRFile file;
    file.open(QFile::encodeName(QDir::toNativeSeparators(fileHandle().localFilePath())).constData());

    Frame frame(fileHandle());
    while(!file.eof() && !isCanceled()) {
        frame.byteOffset = file.byteOffset();
        if(!setProgressValue(frame.byteOffset))
            return;

        // Parse trajectory frame.
        TRRFile::Frame trrFrame = file.read();
        if(file.eof()) break;

        // Create a new record for the timestep.
        frame.label = tr("Timestep %1").arg(trrFrame.step);
        frames.push_back(frame);
    }
}

/******************************************************************************
* Parses the given input file.
******************************************************************************/
void TRRImporter::FrameLoader::loadFile()
{
    // Open file for reading.
    setProgressText(tr("Reading TRR file %1").arg(fileHandle().toString()));

    // Open TRR file for reading.
    TRRFile file;
    file.open(QFile::encodeName(QDir::toNativeSeparators(fileHandle().localFilePath())).constData());

    // Seek to byte offset of requested trajectory frame.
    if(frame().byteOffset != 0)
        file.seek(frame().byteOffset);

    // Read trajectory frame data.
    TRRFile::Frame trrFrame = file.read();

    // Transfer atomic coordinates to property storage. Also convert from nanometer units to angstroms.
    size_t numParticles = trrFrame.xyz.size();
    setParticleCount(numParticles);
    BufferWriteAccess<Point3, access_mode::discard_write> posProperty = particles()->createProperty(Particles::PositionProperty);
    std::transform(trrFrame.xyz.cbegin(), trrFrame.xyz.cend(), posProperty.begin(), [](const Point_3<float>& p) {
        return (p * 10.0f).toDataType<FloatType>();
    });
    posProperty.reset();

    // if velocity, unit = nm/ps
    if (trrFrame.has_prop & HASV)
    {
        BufferWriteAccess<Point3, access_mode::discard_write> velProperty = particles()->createProperty(Particles::VelocityProperty);
        std::transform(trrFrame.vel.cbegin(), trrFrame.vel.cend(), velProperty.begin(), [](const Point_3<float>& p) {
            return p.toDataType<FloatType>();
            });
        velProperty.reset();
    }

    // if force, unit = kj/mol/nm
    if (trrFrame.has_prop & HASF)
    {
        BufferWriteAccess<Point3, access_mode::discard_write> forceProperty = particles()->createProperty(Particles::ForceProperty);
        std::transform(trrFrame.force.cbegin(), trrFrame.force.cend(), forceProperty.begin(), [](const Point_3<float>& p) {
            return p.toDataType<FloatType>();
            });
        forceProperty.reset();
    }

    // Convert cell vectors from nanometers to angstroms.
    simulationCell()->setCellMatrix(AffineTransformation((trrFrame.cell * 10.0f).toDataType<FloatType>()));

    state().setAttribute(QStringLiteral("Timestep"), QVariant::fromValue(trrFrame.step), pipelineNode());
    state().setAttribute(QStringLiteral("Time"), QVariant::fromValue((FloatType)trrFrame.time), pipelineNode());
    
    // show some info
    state().setStatus(
        tr("Total %1 atoms\nUnits: coords (Angstrom), velocity (nm/ps), force (kJ/mol/nm)")
        .arg(numParticles)
    );

    // Call base implementation to finalize the loaded particle data.
    ParticleImporter::FrameLoader::loadFile();
}

}   // End of namespace
