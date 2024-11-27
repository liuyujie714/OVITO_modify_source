#pragma once

#include <ovito/particles/Particles.h>
#include <ovito/particles/import/ParticleImporter.h>
#include <ovito/particles/objects/Particles.h>
#include <ovito/core/dataset/DataSetContainer.h>

namespace Ovito {

class OVITO_PARTICLES_EXPORT TprImporter : public ParticleImporter
{
private:
    /// Defines a metaclass specialization for this importer type.
    class OOMetaClass : public ParticleImporter::OOMetaClass
    {
    public:
        /// Inherit standard constructor from base meta class.
        using ParticleImporter::OOMetaClass::OOMetaClass;

        /// Returns the list of file formats that can be read by this importer class.
        virtual Ovito::span<const SupportedFormat> supportedFormats() const override {
            static const SupportedFormat formats[] = {{ QStringLiteral("*.tpr"), tr("Gromacs Tpr Files") }};
            return formats;
        }

        /// Checks if the given file has format that can be read by this importer.
        virtual bool checkFileFormat(const FileHandle& file) const override;
    };

    OVITO_CLASS_META(TprImporter, OOMetaClass)

    
public:
    /// \brief Constructs a new instance of this class.
    Q_INVOKABLE TprImporter(ObjectInitializationFlags flags) : ParticleImporter(flags) {
        setRecenterCell(true);
    }

    /// Returns the title of this object.
    virtual QString objectTitle() const override { return tr("TPR"); }

    /// Creates an asynchronous loader object that loads the data for the given frame from the external file.
    virtual FileSourceImporter::FrameLoaderPtr createFrameLoader(const LoadOperationRequest& request) override {
        activateCLocale();
        return std::make_shared<FrameLoader>(request, recenterCell());
    }

private:
    /// The format-specific task object that is responsible for reading an input file in the background.
    class FrameLoader : public ParticleImporter::FrameLoader
    {
    public:
        /// Constructor.
        using ParticleImporter::FrameLoader::FrameLoader;

    protected:
        /// Reads the frame data from the external file.
        virtual void loadFile() override;
    };
};


} // end of namespace Ovito
