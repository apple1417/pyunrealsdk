#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/gbxdefptr.h"
#include "unrealsdk/unreal/classes/properties/fgbxdefptrproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_gbxdefptr(py::module_& mod) {
    py::class_<FGbxDefPtr>(
        mod, "GbxDefPtr",
        "THIS TYPE IS STILL EXPERIMENTAL\n"
        "\n"
        "It may currently leak memory and/or cause crashes, and it's semantics are all\n"
        "liable to change.")
        .def(py::init<>(), "Creates a default (empty) GbxDefPtr.\n")
        .def_readwrite("_experimental_name", &FGbxDefPtr::name, "The name of this definition.")
        .def_readwrite("_experimental_ref", &FGbxDefPtr::ref,
                       "The script struct type this refers to.")
        .def_property_readonly(
            "_experimental_instance",
            [](const FGbxDefPtr& self) -> std::optional<WrappedStruct> {
                if (self.ref == nullptr || self.instance == nullptr) {
                    return std::nullopt;
                }
                return WrappedStruct{self.ref, self.instance};
            },
            "The instance data as a WrappedStruct, or None if not available.")
        .def("__repr__", [](const FGbxDefPtr& self) {
            std::string name_str = self.name;
            return std::format(
                "GbxDefPtr('{}', {})", self.name,
                (self.ref == nullptr || self.instance == nullptr) ? "None" : "{...}");
        });
}

}  // namespace pyunrealsdk::unreal

#endif
