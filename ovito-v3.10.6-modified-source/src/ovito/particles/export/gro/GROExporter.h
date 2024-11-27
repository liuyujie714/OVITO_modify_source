#pragma once

#include <ovito/particles/Particles.h>
#include "../ParticleExporter.h"

namespace Ovito {

/* \brief Exporter that writes the particles to a GRO data file. */
class OVITO_PARTICLES_EXPORT GROExporter : public ParticleExporter
{
    /// Defines a metaclass specialization for this exporter type.
    class OOMetaClass : public ParticleExporter::OOMetaClass
    {
    public:
        /// Inherit standard constructor from base meta class.
        using ParticleExporter::OOMetaClass::OOMetaClass;

        /// Returns the file filter that specifies the extension of files written by this service.
        virtual QString fileFilter() const override { return QStringLiteral("*"); }

        /// Returns the filter description that is displayed in the drop-down box of the file dialog.
        virtual QString fileFilterDescription() const override { return tr("GRO File"); }
    };

    OVITO_CLASS_META(GROExporter, OOMetaClass)

public:
    /// \brief Constructs a new instance of this class.
    Q_INVOKABLE GROExporter(ObjectInitializationFlags flags) : ParticleExporter(flags) {}

    /// \brief Indicates whether this file exporter can write more than one animation frame into a single output file.
    virtual bool supportsMultiFrameFiles() const override { return true; }

protected:
    /// \brief Writes the particles of one animation frame to the current output file.
    virtual bool exportData(const PipelineFlowState& state, int frameNumber, const QString& filePath,
                            MainThreadOperation& operation) override;

};

} // namespace Ovito
