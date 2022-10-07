//
// Python Node
//

#ifndef FLOWCV_PYTHON_NODE_HPP_
#define FLOWCV_PYTHON_NODE_HPP_
#include <DSPatch.h>
#include "FlowCV_Types.hpp"
#include <ImGuiFileBrowser.h>
#include "python_node_data.hpp"

namespace DSPatch::DSPatchables
{
namespace internal
{
class PythonNode;
}

class DLLEXPORT PythonNode final : public Component
{
  public:
    PythonNode();
    ~PythonNode() override;
    void UpdateGui(void *context, int interface) override;
    bool HasGui(int interface) override;
    std::string GetState() override;
    void SetState(std::string &&json_serialized) override;

  protected:
    void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

  private:
    std::unique_ptr<internal::PythonNode> p;
    std::mutex io_mutex_;
    std::shared_ptr<pyNodeDataWrapper> py_data_;
    std::string py_script_path_;
    bool show_script_file_dialog_;
    bool show_interpret_dialog_;
    bool file_exists_;
    imgui_addons::ImGuiFileBrowser script_file_dialog_;
    imgui_addons::ImGuiFileBrowser iterpret_file_dialog_;
};

EXPORT_PLUGIN( PythonNode )

}  // namespace DSPatch::DSPatchables

#endif //FLOWCV_PYTHON_NODE_HPP_
