//
// Python Node Data Handler Classes
//

#include "python_node_data.hpp"

#ifdef __linux__
#include <dlfcn.h>
#endif

cv::Mat* pyNodeDataWrapper::get_image(int io_num)
{
    if (io_num == 1)
        return &img2_;

    return &img1_;
}

void pyNodeDataWrapper::set_image(py::array_t<uint8_t>& img, int io_num)
{
    cv::Mat mat(img.shape(0), img.shape(1), CV_MAKETYPE(CV_8U, img.shape(2)),
                const_cast<uint8_t*>(img.data()), img.strides(0));

    if (io_num == 0)
        mat.copyTo(img1_);
    else if (io_num == 1)
        mat.copyTo(img2_);
}

void pyNodeDataWrapper::set_json(const nl::json& j)
{
    json_data_ = nl::json(j);
}

nl::json* pyNodeDataWrapper::get_json()
{
    return &json_data_;
}

bool pyNodeDataWrapper::get_bool() {
    return bool_val_;
}

void pyNodeDataWrapper::set_bool(bool val) {
    bool_val_ = val;
}

int pyNodeDataWrapper::get_int() {
    return int_val_;
}

void pyNodeDataWrapper::set_int(int val) {
    int_val_ = val;
}

float pyNodeDataWrapper::get_float() {
    return float_val_;
}

void pyNodeDataWrapper::set_float(float val) {
    float_val_ = val;
}

std::string pyNodeDataWrapper::get_string() {
    return str_val_;
}

void pyNodeDataWrapper::set_string(std::string str) {
    str_val_ = str;
}

std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

pyBindScope::pyBindScope()
{
    is_init_ = false;
    python_env_ = "";
    changed_interpreter_ = false;

#ifdef _WIN32
    char *python_home = nullptr;
    size_t var_len;
    _dupenv_s(&python_home, &var_len, "PYTHONHOME");
    if (var_len != 0) {
        python_env_ = python_home;
    }
    else {
        char *path;
        _dupenv_s(&path, &var_len, "PATH");
        auto path_Items = split(path, ";");
        for (const auto &item : path_Items) {
            std::string lower = item;
            std::for_each(lower.begin(), lower.end(), [](char & c) {
                c = ::tolower(c);
            });
            if (lower.find("python") != std::string::npos) {
                std::string py_exe_path = item;
                py_exe_path += "\\python.exe";
                if (std::filesystem::exists(py_exe_path)) {
                    python_env_ = item;
                    break;
                }
            }
        }
    }
    if (!python_env_.empty())
        InitPythonInterpreter();
#else
    InitPythonInterpreter();
#endif

}

void pyBindScope::InitPythonInterpreter()
{
    if (!is_init_) {
#ifdef _WIN32
        if (!python_env_.empty()) {
            std::string py_home = "PYTHONHOME=" + python_env_;
            _putenv(py_home.c_str());
        }
#endif
        try {
            py::initialize_interpreter();
#ifdef __linux__
            auto sysMod = py::module_::import("sys");
            auto pyVerStr = sysMod.attr("version").cast<std::string>();
            auto pyVerSplit = split(pyVerStr, ".");
            std::string libPyStr = "libpython";
            if (libPyStr.size() > 2) {
                libPyStr += pyVerSplit.at(0);
                libPyStr += ".";
                libPyStr += pyVerSplit.at(1);
                libPyStr += ".so";
                dlopen(libPyStr.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            }
#endif
            scp_rel_ = std::make_shared<py::gil_scoped_release>();
            is_init_ = true;
            return;
        }
        catch (py::error_already_set &e) {
            std::cout << e.what() << std::endl;
        }
        catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
}

