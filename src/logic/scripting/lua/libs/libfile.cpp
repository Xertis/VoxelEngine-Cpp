#include <string>
#include <set>

#include "coders/gzip.hpp"
#include "engine/Engine.hpp"
#include "io/engine_paths.hpp"
#include "io/io.hpp"
#include "io/devices/ZipFileDevice.hpp"
#include "util/stringutil.hpp"
#include "api_lua.hpp"
#include "../lua_engine.hpp"
#include "logic/scripting/descriptors_manager.hpp"

namespace fs = std::filesystem;
using namespace scripting;

static int l_find(lua::State* L) {
    auto path = lua::require_string(L, 1);
    try {
        return lua::pushstring(L, engine->getResPaths().findRaw(path));
    } catch (const std::runtime_error& err) {
        return 0;
    }
}

static int l_resolve(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    return lua::pushstring(L, path.string());
}

static int l_read(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    if (io::is_regular_file(path)) {
        return lua::pushlstring(L, io::read_string(path));
    }
    throw std::runtime_error(
        "file does not exists " + util::quote(path.string())
    );
}

static std::set<std::string> writeable_entry_points {
    "world", "export", "config"
};

static bool is_writeable(const std::string& entryPoint) {
    if (entryPoint.length() < 2) {
        return false;
    }
    if (entryPoint.substr(0, 2) == "W.") {
        return true;
    }
    if (writeable_entry_points.find(entryPoint) != writeable_entry_points.end()) {
        return true;
    }
    return false;
}

static io::path get_writeable_path(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    auto entryPoint = path.entryPoint();
    if (!is_writeable(entryPoint)) {
        throw std::runtime_error("access denied");
    }
    return path;
}

static int l_write(lua::State* L) {
    io::path path = get_writeable_path(L);
    std::string text = lua::require_string(L, 2);
    io::write_string(path, text);
    return 1;
}

static int l_remove(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    auto entryPoint = path.entryPoint();
    if (!is_writeable(entryPoint)) {
        throw std::runtime_error("access denied");
    }
    return lua::pushboolean(L, io::remove(path));
}

static int l_remove_tree(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    auto entryPoint = path.entryPoint();
    if (!is_writeable(entryPoint)) {
        throw std::runtime_error("access denied");
    }
    return lua::pushinteger(L, io::remove_all(path));
}

static int l_exists(lua::State* L) {
    return lua::pushboolean(L, io::exists(lua::require_string(L, 1)));
}

static int l_isfile(lua::State* L) {
    return lua::pushboolean(L, io::is_regular_file(lua::require_string(L, 1)));
}

static int l_isdir(lua::State* L) {
    return lua::pushboolean(L, io::is_directory(lua::require_string(L, 1)));
}

static int l_length(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    if (io::exists(path)) {
        return lua::pushinteger(L, io::file_size(path));
    } else {
        return lua::pushinteger(L, -1);
    }
}

static int l_mkdir(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    return lua::pushboolean(L, io::create_directory(path));
}

static int l_mkdirs(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    return lua::pushboolean(L, io::create_directories(path));
}

static int l_read_bytes(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    if (io::is_regular_file(path)) {
        size_t length = static_cast<size_t>(io::file_size(path));

        auto bytes = io::read_bytes(path);

        if (lua::gettop(L) < 2 || !lua::toboolean(L, 2)) {
            lua::create_bytearray(L, std::move(bytes));
        } else {
            lua::createtable(L, length, 0);
            int newTable = lua::gettop(L);

            for (size_t i = 0; i < length; i++) {
                lua::pushinteger(L, bytes[i]);
                lua::rawseti(L, i + 1, newTable);
            }
        }
        return 1;
    }
    throw std::runtime_error(
        "file does not exists " + util::quote(path.string())
    );
}

static int l_write_bytes(lua::State* L) {
    io::path path = get_writeable_path(L);

    auto string = lua::bytearray_as_string(L, 2);
    bool res = io::write_bytes(
        path, reinterpret_cast<const ubyte*>(string.data()), string.size()
    );
    lua::pop(L);
    return lua::pushboolean(L, res);
}

static int l_list_all_res(lua::State* L) {
    std::string path = lua::require_string(L, 1);
    auto files = engine->getResPaths().listdirRaw(path);
    lua::createtable(L, files.size(), 0);
    for (size_t i = 0; i < files.size(); i++) {
        lua::pushstring(L, files[i]);
        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_list(lua::State* L) {
    std::string dirname = lua::require_string(L, 1);
    if (dirname.find(':') == std::string::npos) {
        return l_list_all_res(L);
    }
    io::path path = dirname;
    if (!io::is_directory(path)) {
        throw std::runtime_error(
            util::quote(path.string()) + " is not a directory"
        );
    }
    lua::createtable(L, 0, 0);
    size_t index = 1;
    for (const auto& file : io::directory_iterator(path)) {
        lua::pushstring(L, file.string());
        lua::rawseti(L, index);
        index++;
    }
    return 1;
}

static int l_gzip_compress(lua::State* L) {
    char argc = lua::gettop(L);
    auto str = lua::bytearray_as_string(L, 1);
    auto compressedBytes = gzip::compress(
        reinterpret_cast<const ubyte*>(str.data()),
        str.size()
    );

    if (argc < 2 || !lua::toboolean(L, 2)) {
        lua::create_bytearray(L, std::move(compressedBytes));
    } else {
        size_t length = compressedBytes.size();
        lua::createtable(L, length, 0);
        int newTable = lua::gettop(L);
        for (size_t i = 0; i < length; i++) {
            lua::pushinteger(L, compressedBytes.data()[i]);
            lua::rawseti(L, i + 1, newTable);
        }
    }

    return 1;
}

static int l_gzip_decompress(lua::State* L) {
    char argc = lua::gettop(L);
    auto str = lua::bytearray_as_string(L, 1);
    auto decompressedBytes = gzip::decompress(
        reinterpret_cast<const ubyte*>(str.data()),
        str.size()
    );

    if (argc < 2 || !lua::toboolean(L, 2)) {
        lua::create_bytearray(L, std::move(decompressedBytes));
    } else {
        size_t length = decompressedBytes.size();
        lua::createtable(L, length, 0);
        int newTable = lua::gettop(L);
        for (size_t i = 0; i < length; i++) {
            lua::pushinteger(L, decompressedBytes.data()[i]);
            lua::rawseti(L, i + 1, newTable);
        }
    }

    return 1;
}

static int l_read_combined_list(lua::State* L) {
    std::string path = lua::require_string(L, 1);
    if (path.find(':') != std::string::npos) {
        throw std::runtime_error("entry point must not be specified");
    }
    return lua::pushvalue(L, engine->getResPaths().readCombinedList(path));
}

static int l_read_combined_object(lua::State* L) {
    std::string path = lua::require_string(L, 1);
    if (path.find(':') != std::string::npos) {
        throw std::runtime_error("entry point must not be specified");
    }
    return lua::pushvalue(L, engine->getResPaths().readCombinedObject(path));
}

static int l_is_writeable(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    auto entryPoint = path.entryPoint();
    return lua::pushboolean(L, is_writeable(entryPoint));
}

static int l_mount(lua::State* L) {
    auto& paths = engine->getPaths();
    return lua::pushstring(L, paths.mount(lua::require_string(L, 1)));
}

static int l_unmount(lua::State* L) {
    auto& paths = engine->getPaths();
    paths.unmount(lua::require_string(L, 1));
    return 0;
}

static int l_create_zip(lua::State* L) {
    io::path folder = lua::require_string(L, 1);
    io::path outFile = lua::require_string(L, 2);
    if (!is_writeable(outFile.entryPoint())) {
        throw std::runtime_error("access denied");
    }
    io::write_zip(folder, outFile);
    return 0;
}

static int l_open_descriptor(lua::State* L) {
    io::path path = lua::require_string(L, 1);
    auto mode = lua::require_lstring(L, 2);

    bool write = mode.find('w') != std::string::npos;
    bool read = mode.find('r') != std::string::npos;

    if (write && !is_writeable(path.entryPoint())) {
        throw std::runtime_error("access denied");
    }

    if(!write && !read) {
        throw std::runtime_error("mode must contain read or write flag");
    }

    if(write && read) {
        throw std::runtime_error("random access file i/o is not supported");
    }

    bool wplusMode = write && mode.find('+') != std::string::npos;

    std::vector<char> buffer;

    if(wplusMode) {
        int temp_descriptor = scripting::descriptors_manager::open_descriptor(path, false, true);

        if (temp_descriptor == -1) {
            throw std::runtime_error("failed to open descriptor for initial reading");
        }

        auto* in_stream = scripting::descriptors_manager::get_input(temp_descriptor);

        in_stream->seekg(0, std::ios::end);
        std::streamsize size = in_stream->tellg();
        in_stream->seekg(0, std::ios::beg);

        buffer.resize(size);
        in_stream->read(buffer.data(), size);

        scripting::descriptors_manager::close(temp_descriptor);
    }

    int descriptor = scripting::descriptors_manager::open_descriptor(path, write, read);

    if(descriptor == -1) {
        throw std::runtime_error("failed to open descriptor");
    }

    if(wplusMode) {
        auto* out_stream = scripting::descriptors_manager::get_output(descriptor);
        out_stream->write(buffer.data(), buffer.size());
        out_stream->flush();
    }

    return lua::pushinteger(L, descriptor);
}

static int l_has_descriptor(lua::State* L) {
    return lua::pushboolean(L, scripting::descriptors_manager::has_descriptor(lua::tointeger(L, 1)));
}

static int l_read_descriptor(lua::State* L) {
    int descriptor = lua::tointeger(L, 1);

    if (!scripting::descriptors_manager::has_descriptor(descriptor)) {
        throw std::runtime_error("unknown descriptor");
    }

    if (!scripting::descriptors_manager::is_readable(descriptor)) {
        throw std::runtime_error("descriptor is not readable");
    }

    int maxlen = lua::tointeger(L, 2);

    auto* stream = scripting::descriptors_manager::get_input(descriptor);

    util::Buffer<char> buffer(maxlen);

    stream->read(buffer.data(), maxlen);

    std::streamsize read_len = stream->gcount(); 

    return lua::create_bytearray(L, buffer.data(), read_len);
}

static int l_write_descriptor(lua::State* L) {
    int descriptor = lua::tointeger(L, 1);

    if (!scripting::descriptors_manager::has_descriptor(descriptor)) {
        throw std::runtime_error("unknown descriptor");
    }

    if (!scripting::descriptors_manager::is_writeable(descriptor)) {
        throw std::runtime_error("descriptor is not writeable");
    }

    auto data = lua::bytearray_as_string(L, 2);

    auto* stream = scripting::descriptors_manager::get_output(descriptor);

    stream->write(data.data(), static_cast<std::streamsize>(data.size()));

    if (!stream->good()) {
        throw std::runtime_error("failed to write to stream");
    }

    return 0;
}

static int l_flush_descriptor(lua::State* L) {
    int descriptor = lua::tointeger(L, 1);

    if (!scripting::descriptors_manager::has_descriptor(descriptor)) {
        throw std::runtime_error("unknown descriptor");
    }

    if (!scripting::descriptors_manager::is_writeable(descriptor)) {
        throw std::runtime_error("descriptor is not writeable");
    }

    scripting::descriptors_manager::flush(descriptor);

    return 0;
}

static int l_close_descriptor(lua::State* L) {
    int descriptor = lua::tointeger(L, 1);

    if (!scripting::descriptors_manager::has_descriptor(descriptor)) {
        throw std::runtime_error("unknown descriptor");
    }

    scripting::descriptors_manager::close(descriptor);

    return 0;
}

static int l_close_all_descriptors(lua::State* L) {
    scripting::descriptors_manager::close_all_descriptors();

    return 0;
}

const luaL_Reg filelib[] = {
    {"exists", lua::wrap<l_exists>},
    {"find", lua::wrap<l_find>},
    {"isdir", lua::wrap<l_isdir>},
    {"isfile", lua::wrap<l_isfile>},
    {"length", lua::wrap<l_length>},
    {"list", lua::wrap<l_list>},
    {"list_all_res", lua::wrap<l_list_all_res>},
    {"mkdir", lua::wrap<l_mkdir>},
    {"mkdirs", lua::wrap<l_mkdirs>},
    {"read_bytes", lua::wrap<l_read_bytes>},
    {"read", lua::wrap<l_read>},
    {"remove", lua::wrap<l_remove>},
    {"remove_tree", lua::wrap<l_remove_tree>},
    {"resolve", lua::wrap<l_resolve>},
    {"write_bytes", lua::wrap<l_write_bytes>},
    {"write", lua::wrap<l_write>},
    {"gzip_compress", lua::wrap<l_gzip_compress>},
    {"gzip_decompress", lua::wrap<l_gzip_decompress>},
    {"read_combined_list", lua::wrap<l_read_combined_list>},
    {"read_combined_object", lua::wrap<l_read_combined_object>},
    {"is_writeable", lua::wrap<l_is_writeable>},
    {"mount", lua::wrap<l_mount>},
    {"unmount", lua::wrap<l_unmount>},
    {"create_zip", lua::wrap<l_create_zip>},
    {"__open_descriptor", lua::wrap<l_open_descriptor>},
    {"__has_descriptor", lua::wrap<l_has_descriptor>},
    {"__read_descriptor", lua::wrap<l_read_descriptor>},
    {"__write_descriptor", lua::wrap<l_write_descriptor>},
    {"__flush_descriptor", lua::wrap<l_flush_descriptor>},
    {"__close_descriptor", lua::wrap<l_close_descriptor>},
    {"__close_all_descriptors", lua::wrap<l_close_all_descriptors>},
    {NULL, NULL}
};

