#pragma once

#include <filesystem>
#include <iterator>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "typedefs.hpp"
#include "data/dv.hpp"
#include "util/Buffer.hpp"
#include "path.hpp"

namespace io {
    class Device;

    void set_device(const std::string& name, std::shared_ptr<Device> device);
    std::shared_ptr<Device> get_device(const std::string& name);
    Device& require_device(const std::string& name);

    void create_subdevice(
        const std::string& name, const std::string& parent, const path& root
    );

    /// @brief Read-only random access file
    class rafile {
        std::ifstream file;
        size_t filelength;
    public:
        rafile(const path& filename);

        void seekg(std::streampos pos);
        void read(char* buffer, std::streamsize size);
        size_t length() const;
    };

    class directory_iterator_impl {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = path;
        using difference_type = std::ptrdiff_t;
        using pointer = path*;
        using reference = path&;

        directory_iterator_impl(
            PathsGenerator& generator, const path& folder, bool end = false
        )
            : generator(generator), folder(folder), isend(end) {
            if (!isend && this->generator.next(current)) {
                isend = false;
                current = folder / current;
            } else {
                isend = true;
            }
        }

        reference operator*() {
            return current;
        }

        pointer operator->() {
            return &current;
        }

        directory_iterator_impl& operator++() {
            if (isend) {
                return *this;
            }
            if (generator.next(current)) {
                current = folder / current;
            } else {
                isend = true;
            }
            return *this;
        }

        bool operator==(const directory_iterator_impl& other) const {
            return isend == other.isend;
        }

        bool operator!=(const directory_iterator_impl& other) const {
            return !(*this == other);
        }
    private:
        PathsGenerator& generator;
        path folder;
        path current;
        bool isend = false;
    };

    class directory_iterator {
        std::unique_ptr<PathsGenerator> generator;
        path folder;
    public:
        directory_iterator(const path& folder);

        directory_iterator_impl begin() {
            return directory_iterator_impl(*generator, folder);
        }

        directory_iterator_impl end() {
            return directory_iterator_impl(*generator, "", true);
        }
    };

    /// @brief Write bytes array to the file without any extra data
    /// @param file target file
    /// @param data data bytes array
    /// @param size size of data bytes array
    bool write_bytes(const io::path& file, const ubyte* data, size_t size);

    /// @brief Write string to the file
    bool write_string(const io::path& file, std::string_view content);

    /// @brief Write dynamic data to the JSON file
    /// @param nice if true, human readable format will be used, otherwise
    /// minimal
    bool write_json(
        const io::path& file, const dv::value& obj, bool nice = true
    );

    /// @brief Write dynamic data to the binary JSON file
    /// (see src/coders/binary_json_spec.md)
    /// @param compressed use gzip compression
    bool write_binary_json(
        const io::path& file,
        const dv::value& obj,
        bool compressed = false
    );

    bool read(const io::path& file, char* data, size_t size);
    util::Buffer<ubyte> read_bytes_buffer(const path& file);
    std::unique_ptr<ubyte[]> read_bytes(const path& file, size_t& length);
    std::vector<ubyte> read_bytes(const path& file);
    std::string read_string(const path& file);

    /// @brief Read JSON or BJSON file
    /// @param file *.json or *.bjson file
    dv::value read_json(const path& file);
    
    dv::value read_binary_json(const path& file);
    
    /// @brief Read TOML file
    /// @param file *.toml file
    dv::value read_toml(const path& file);

    std::vector<std::string> read_list(const io::path& file);

    bool is_regular_file(const io::path& file);
    bool is_directory(const io::path& file);
    bool exists(const io::path& file);
    bool create_directories(const io::path& file);
    bool remove(const io::path& file);
    uint64_t remove_all(const io::path& file);
    size_t file_size(const io::path& file);

    std::filesystem::path resolve(const io::path& file);

    /// @brief Check if file is one of the supported data interchange formats
    bool is_data_file(const io::path& file);

    /// @brief Check if file extension is one of the supported data interchange formats
    bool is_data_interchange_format(const std::string& ext);
    
    dv::value read_object(const path& file);
}
