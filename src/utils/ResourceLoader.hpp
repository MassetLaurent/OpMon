/*!
 * \file ResourceLoader.hpp
 * \author BAKFR
 * \copyright GNU GPL v3.0
 */
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cstddef>
#include <fstream>
#include <memory>
#include <vector>

#include "exceptions.hpp"
#include "log.hpp"

namespace sf {
    class Music;
    class Texture;
}  // namespace sf

namespace Utils {

    class ResourceLoader {
    public:
        static std::string getResourcePath();

        static void setResourcePath(std::string path);

        /*!
         * \brief Verifies that the resource folder exists.
         * \returns `true` if it exists; `false` otherwise.
         */
        static bool checkResourceFolderExists();

        /*!
         * \brief Loads an arbitrary SFML resource
         *
         * \tparam T - the method T::loadFromFile() must exists.
         * \param resource - the resource to load
         * \param path - path of the resource, relative to the resource folder.
         * \param fatal - if true, the program quit if there is an error.
         */
        template <typename T>
        static void load(T &resource, std::string path, bool fatal = false);

        /*!
         * \brief Loads an array of textures (multiple frames of the same animation).
         *
         * \param container
         * \param path - path relative to the resource folder. It must contains a "%d" , which will be replaced by the
         *    frame number.
         * \param nb_frame - number of texture to load.
         * \param path_offset - by default, the first frame number is 0. If set, the first frame number will be the
         *    offset.
         */
        static void loadTextureArray(sf::Texture container[], std::string path, size_t nb_frame, size_t path_offset = 0);

        /*!
         * \copydoc loadTextureArray(sf::Texture container[], const char *path, size_t nb_frame, size_t path_offset = 0)
         *
         * The container must be empty.
         */
        static void loadTextureArray(std::vector<sf::Texture> &container, std::string path, size_t nb_frame, size_t path_offset = 0);

        /*!
         * \brief Loads a music.
         * \param path The path to the music file.
         */
        static std::unique_ptr<sf::Music> loadMusic(const char *path);

    private:
        static std::string resourcePath;

    };

    template <typename T>
    void ResourceLoader::load(T &resource, std::string path, bool fatal) {
        try{
            if(!resource.loadFromFile(ResourceLoader::getResourcePath() + path)) {
                throw LoadingException(path, fatal);
            }
        } catch (LoadingException& e) {
            if(e.fatal) throw;
            else Log::warn(e.desc());
        }
    }

} // namespace Utils
