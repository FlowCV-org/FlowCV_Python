//
// Python Node
//

#include "python_node.hpp"
#include "imgui.h"
#include "imgui_instance_helper.hpp"
#include <unordered_map>

using namespace DSPatch;
using namespace DSPatchables;
namespace py = pybind11;
using namespace py::literals;

int32_t global_inst_counter = 0;

std::unordered_map<std::string, std::shared_ptr<pyNodeDataWrapper>> node_data;

std::shared_ptr<pyNodeDataWrapper> get_data(std::string& active_name)
{
    if(node_data.find(active_name) != node_data.end())
        return node_data[active_name];
    else
        return nullptr;
}

PYBIND11_EMBEDDED_MODULE(flowcv, m)
{
    m.doc() = "FlowCV Python Node Interface";
    m.def("node_data", &get_data);
    py::class_<pyNodeDataWrapper, std::shared_ptr<pyNodeDataWrapper>>(m, "flowData")
        .def(py::init())
        .def("get_image", &pyNodeDataWrapper::get_image, py::return_value_policy::reference_internal, "Get Input Image, 0 = img1, 1 = img2")
        .def("set_image", &pyNodeDataWrapper::set_image, "Set Output Image, 0 = img1, 1 = img2")
        .def("get_json", &pyNodeDataWrapper::get_json, py::return_value_policy::reference_internal, "Get Input JSON Data, Returns None Type If Empty")
        .def("set_json", &pyNodeDataWrapper::set_json, "Set Output JSON Data")
        .def("get_bool", &pyNodeDataWrapper::get_bool,"Get Bool Input Value")
        .def("set_bool", &pyNodeDataWrapper::set_bool,"Set Bool Output Value")
        .def("get_int", &pyNodeDataWrapper::get_int,"Get Int Input Value")
        .def("set_int", &pyNodeDataWrapper::set_int,"Set Int Output Value")
        .def("get_float", &pyNodeDataWrapper::get_float,"Get Float Input Value")
        .def("set_float", &pyNodeDataWrapper::set_float,"Set Float Output Value")
        .def("get_string", &pyNodeDataWrapper::get_string,"Get String Input")
        .def("set_string", &pyNodeDataWrapper::set_string,"Set String Output")
        ;
    py::class_<cv::Mat>(m, "Image", pybind11::buffer_protocol())
        .def_buffer([](cv::Mat& im) -> pybind11::buffer_info {
            return pybind11::buffer_info(
                // Pointer to buffer
                im.data,
                // Size of one scalar
                sizeof(unsigned char),
                // Python struct-style format descriptor
                pybind11::format_descriptor<unsigned char>::format(),
                // Number of dimensions
                3,
                // Buffer dimensions
                { im.rows, im.cols, im.channels() },
                // Strides (in bytes) for each index
                {
                    (size_t)im.step,
                    sizeof(unsigned char) * im.channels(),
                    sizeof(unsigned char)
                }
            );
        });
}

pyBindScope py_bind_;

namespace DSPatch::DSPatchables::internal
{
class PythonNode
{
};
}  // namespace DSPatch

PythonNode::PythonNode()
    : Component( ProcessOrder::InOrder )
    , p( new internal::PythonNode() )
{
    // Name and Category
    SetComponentName_("Python_Script");
    SetComponentCategory_(Category::Category_Experimental);
    SetComponentAuthor_("Richard");
    SetComponentVersion_("0.1.1");
    SetInstanceCount(global_inst_counter);
    global_inst_counter++;

    // 7 inputs
    SetInputCount_( 7, {"img1", "img2", "bool", "int", "float", "str", "json"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_Bool, IoType::Io_Type_Int, IoType::Io_Type_Float, IoType::Io_Type_String, IoType::Io_Type_JSON} );

    // 7 outputs
    SetOutputCount_( 7, {"img1", "img2", "bool", "int", "float", "str", "json"}, {IoType::Io_Type_CvMat, IoType::Io_Type_CvMat, IoType::Io_Type_Bool, IoType::Io_Type_Int, IoType::Io_Type_Float, IoType::Io_Type_String, IoType::Io_Type_JSON} );

    show_script_file_dialog_ = false;
    show_interpret_dialog_ = false;
    file_exists_ = false;
    py_data_ = std::make_shared<pyNodeDataWrapper>();
    py_data_->node_name_ = GetInstanceName();
    node_data[py_data_->node_name_] = py_data_;
    SetEnabled(true);
}

PythonNode::~PythonNode()
{
    auto data_entry = node_data.find(py_data_->node_name_);
    if (data_entry != node_data.end())
        node_data.erase(data_entry);
}

void PythonNode::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    if (io_mutex_.try_lock()) { // Limit each instance to 1 thread to avoid glitches, skip if already locked
        auto in1 = inputs.GetValue<cv::Mat>(0);
        auto in2 = inputs.GetValue<cv::Mat>(1);
        auto in_bool = inputs.GetValue<bool>(2);
        auto in_int = inputs.GetValue<int>(3);
        auto in_float = inputs.GetValue<float>(4);
        auto in_str = inputs.GetValue<std::string>(5);
        auto in_json = inputs.GetValue<nl::json>(6);
        if (in1) {
            if (!in1->empty())
                in1->copyTo(py_data_->img1_in_);
            else
                py_data_->img1_in_ = cv::Mat();
        }
        else
            py_data_->img1_in_ = cv::Mat();

        if (in2) {
            if (!in2->empty())
                in2->copyTo(py_data_->img2_in_);
            else
                py_data_->img2_in_ = cv::Mat();
        }
        else
            py_data_->img2_in_ = cv::Mat();

        if (in_json)
            py_data_->json_data_ = *in_json;
        if (in_bool)
            py_data_->bool_val_ = *in_bool;
        if (in_int)
            py_data_->int_val_ = *in_int;
        if (in_float)
            py_data_->float_val_ = *in_float;
        if (in_str)
            py_data_->str_val_ = *in_str;

        if (!py_script_path_.empty() && file_exists_ && IsEnabled()) {
            if (py_bind_.is_init_) {
                try {
                    py::gil_scoped_acquire gil{};
                    auto locals = py::dict("active_node"_a = py_data_->node_name_);
                    py::eval_file(py_script_path_, py::globals(), locals);
                }
                catch (py::error_already_set &e) {
                    std::cout << e.what() << std::endl;
                }
            }
        }
        if (!py_data_->img1_out_.empty())
            outputs.SetValue(0, py_data_->img1_out_);

        if (!py_data_->img2_out_.empty())
            outputs.SetValue(1, py_data_->img2_out_);
        outputs.SetValue(2, py_data_->bool_val_);
        outputs.SetValue(3, py_data_->int_val_);
        outputs.SetValue(4, py_data_->float_val_);
        if (!py_data_->str_val_.empty())
            outputs.SetValue(5, py_data_->str_val_);
        if (!py_data_->json_data_.empty())
            outputs.SetValue(6, py_data_->json_data_);

        io_mutex_.unlock();
    }
}

bool PythonNode::HasGui(int interface)
{
    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
        return true;
    }

    return false;
}

void PythonNode::UpdateGui(void *context, int interface)
{
    auto *imCurContext = (ImGuiContext *)context;
    ImGui::SetCurrentContext(imCurContext);

    if (interface == (int)FlowCV::GuiInterfaceType_Controls) {
#ifdef _WIN32
        ImGui::Text("GLOBAL");
        if (py_bind_.python_env_.empty())
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Python Interpreter Set/Found");
        else {
            ImGui::Text("Python Interpreter: %s", py_bind_.python_env_.c_str());
        }
        if (ImGui::Button(CreateControlString("Set Python Interpreter", GetInstanceName()).c_str())) {
            show_interpret_dialog_ = true;
        }
        if (py_bind_.is_init_ && py_bind_.changed_interpreter_) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::TextWrapped("Changing the interpreter after init requires a save and restart");
            ImGui::PopStyleColor(1);
        }
        ImGui::Separator();
#endif
        ImGui::Text("Python Script:");
        ImGui::SameLine();
        if (py_script_path_.empty())
            ImGui::TextWrapped("[None]");
        else {
            if (file_exists_) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            ImGui::TextWrapped("%s", py_script_path_.c_str());
            ImGui::PopStyleColor(1);
        }

        if (ImGui::Button(CreateControlString("Select Script", GetInstanceName()).c_str())) {
            show_script_file_dialog_ = true;
        }

        if(show_script_file_dialog_)
            ImGui::OpenPopup(CreateControlString("Script Selection", GetInstanceName()).c_str());

        if (show_interpret_dialog_)
            ImGui::OpenPopup(CreateControlString("Python Interpreter", GetInstanceName()).c_str());

        if(script_file_dialog_.showFileDialog(CreateControlString("Script Selection", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".py", &show_script_file_dialog_))
        {
            py_script_path_ = script_file_dialog_.selected_path;
            file_exists_ = std::filesystem::exists(py_script_path_);
            show_script_file_dialog_ = false;
        }

        if(iterpret_file_dialog_.showFileDialog(CreateControlString("Python Interpreter", GetInstanceName()), imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".exe", &show_interpret_dialog_))
        {
            std::filesystem::path python_path = iterpret_file_dialog_.selected_path;
            py_bind_.python_env_ = python_path.remove_filename().string();
            show_interpret_dialog_ = false;
            if (py_bind_.is_init_)
                py_bind_.changed_interpreter_ = true;
            py_bind_.InitPythonInterpreter();
        }
    }

}

std::string PythonNode::GetState()
{
    using namespace nlohmann;

    json state;

    state["py_script_path"] = py_script_path_;
    if (!py_bind_.python_env_.empty())
        state["python_interpreter"] = py_bind_.python_env_;

    std::string stateSerialized = state.dump(4);

    return stateSerialized;
}

void PythonNode::SetState(std::string &&json_serialized)
{
    using namespace nlohmann;

    json state = json::parse(json_serialized);

    if (state.contains("py_script_path")) {
        if (!state["py_script_path"].empty()) {
            py_script_path_ = state["py_script_path"].get<std::string>();
            file_exists_ = std::filesystem::exists(py_script_path_);
        }
    }
    if (state.contains("python_interpreter")) {
        if (!state["python_interpreter"].empty()) {
            py_bind_.python_env_ = state["python_interpreter"].get<std::string>();
            py_bind_.InitPythonInterpreter();
        }
    }

}
