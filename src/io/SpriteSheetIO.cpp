#include "io/SpriteSheetIO.hpp"
#include "io/PngIO.hpp"
#include <fstream>
#include <filesystem>

namespace pixelforge::io {

bool SpriteSheetIO::save(const animation::Animation& anim,
                         const std::string& png_filepath,
                         const animation::SpriteSheetOptions& options,
                         const std::string& json_metadata_path) {
    auto res = animation::SpriteSheet::generate(anim, options);
    bool png_ok = PngIO::save(res.sheet_sprite, png_filepath);
    if (!png_ok) return false;

    std::string meta_path = json_metadata_path;
    if (meta_path.empty()) {
        std::filesystem::path p(png_filepath);
        p.replace_extension(".json");
        meta_path = p.string();
    }

    std::ofstream out(meta_path);
    if (out.is_open()) {
        out << res.metadata.dump(2);
    }

    return true;
}

} // namespace pixelforge::io
