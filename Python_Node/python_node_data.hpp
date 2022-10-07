//
// Python Node Data Handler Classes
//

#ifndef FLOWCV_PYTHON_NODE_DATA_HPP_
#define FLOWCV_PYTHON_NODE_DATA_HPP_
#include <iostream>
#include <pybind11_json/pybind11_json.hpp>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "opencv2/opencv.hpp"
#include <vector>
#include <cstdlib>
#include <filesystem>
#include <string>

class pyNodeDataWrapper {
  public:
    pyNodeDataWrapper() = default;
    cv::Mat*  get_image(int io_num = 0);
    void set_image(py::array_t<uint8_t>& img, int io_num = 0);
    nl::json* get_json();
    void set_json(const nl::json& j);
    bool get_bool();
    void set_bool(bool val);
    int get_int();
    void set_int(int val);
    float get_float();
    void set_float(float val);
    std::string get_string();
    void set_string(std::string str);
    cv::Mat img1_;
    cv::Mat img2_;
    bool bool_val_{};
    int int_val_{};
    float float_val_{};
    std::string str_val_;
    nl::json json_data_;
    std::string node_name_;
};

class pyBindScope {
  public:
    pyBindScope();
    void InitPythonInterpreter();
    std::shared_ptr<py::gil_scoped_release> scp_rel_;
    std::mutex io_mutex_;
    std::string python_env_;
    bool changed_interpreter_;
    bool is_init_;
};

#endif //FLOWCV_PYTHON_NODE_DATA_HPP_
