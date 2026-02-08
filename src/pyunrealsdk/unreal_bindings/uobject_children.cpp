#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uobject_children.h"
#include "pyunrealsdk/stubgen.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/unreal/classes/properties/attribute_property.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/fgbxdefptrproperty.h"
#include "unrealsdk/unreal/classes/properties/persistent_object_ptr_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/classes/properties/ubyteproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/ucomponentproperty.h"
#include "unrealsdk/unreal/classes/properties/udelegateproperty.h"
#include "unrealsdk/unreal/classes/properties/uenumproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/umulticastdelegateproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/properties/utextproperty.h"
#include "unrealsdk/unreal/classes/properties/uweakobjectproperty.h"
#include "unrealsdk/unreal/classes/ublueprintgeneratedclass.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uconst.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"
#include "unrealsdk/unreal/structs/ffield.h"
#include "unrealsdk/unreal/structs/tfieldvariant.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_uobject_children(py::module_& mod) {
    PYUNREALSDK_STUBGEN_MODULE_N("unrealsdk.unreal")

    // ======== Not technically subclasses but still closely related ========

    PyUEClass<FFieldClass>(mod, PYUNREALSDK_STUBGEN_CLASS("FFieldClass", ))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Name", "str"), &FFieldClass::Name)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("SuperField", "FFieldClass | None"),
                         &FFieldClass::SuperField)
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__new__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__init__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](FFieldClass* self) {
                // Make this kind of look like a normal class?
                return std::format("FFieldClass'{}'", self->Name());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's full name.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's name.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"),
            [](FFieldClass* self) { return reinterpret_cast<uintptr_t>(self); },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets the address of this object, for debugging.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's address.\n"));

    PyUEClass<FField>(mod, PYUNREALSDK_STUBGEN_CLASS("FField", ))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Class", "UClass"), &FField::Class)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Next", "FField"), &FField::Next)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Name", "str"), &FField::Name)
        .def_property(
            PYUNREALSDK_STUBGEN_ATTR("Owner", "FField | UObject | None"),
            [](FField& self) {
                py::object ret;
                self.Owner().cast([&]<typename T>(T* obj) { ret = py::cast(obj); });
                return ret;
            },
            [](FField& self, std::variant<std::nullptr_t, UObject*, FField*> val) {
                if (std::holds_alternative<std::nullptr_t>(val)) {
                    self.Owner() = nullptr;
                } else if (std::holds_alternative<UObject*>(val)) {
                    self.Owner() = std::get<UObject*>(val);
                } else {
                    self.Owner() = std::get<FField*>(val);
                }
            })
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__new__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(PYUNREALSDK_STUBGEN_NEVER_METHOD("__init__"),
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(
            PYUNREALSDK_STUBGEN_METHOD("__repr__", "str"),
            [](FField* self) {
                return std::format("{}'{}'", self->Class()->Name(),
                                   unrealsdk::utils::narrow(self->get_path_name()));
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's full name.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's name.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_get_address", "int"),
            [](FField* self) { return reinterpret_cast<uintptr_t>(self); },
            PYUNREALSDK_STUBGEN_DOCSTRING("Gets the address of this object, for debugging.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    This object's address.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("_path_name", "str"), &FField::get_path_name,
             PYUNREALSDK_STUBGEN_DOCSTRING("Gets this object's path name, excluding the class.\n"
                                           "\n"
                                           "Returns:\n"
                                           "    This object's name.\n"));

    // ======== First Layer Subclasses ========

    PyUEClass<UField, UObject>(mod, PYUNREALSDK_STUBGEN_CLASS("UField", "UObject"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Next", "UField | None"), &UField::Next);

    // ======== Second Layer Subclasses ========

    PyUEClass<UConst, UField>(mod, PYUNREALSDK_STUBGEN_CLASS("UConst", "UField"))
        // Deliberately not using def_member_prop, since we need to do extra string conversions
        .def_property(
            PYUNREALSDK_STUBGEN_ATTR("Value", "str"),
            [](const UConst* self) { return (std::string)self->Value(); },
            [](UConst* self, const std::string& new_value) { self->Value() = new_value; });

#if UNREALSDK_PROPERTIES_ARE_FFIELD
    PyUEClass<UProperty, FField>(mod, PYUNREALSDK_STUBGEN_CLASS("UProperty", "FField"))
#else
    PyUEClass<UProperty, UField>(mod, PYUNREALSDK_STUBGEN_CLASS("UProperty", "UField"))
#endif
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ArrayDim", "int"), &UProperty::ArrayDim)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ElementSize", "int"), &UProperty::ElementSize)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyFlags", "int"),
                         &UProperty::PropertyFlags)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Offset_Internal", "int"),
                         &UProperty::Offset_Internal)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyLinkNext", "UProperty | None"),
                         &UProperty::PropertyLinkNext);

    PyUEClass<UStruct, UField>(mod, PYUNREALSDK_STUBGEN_CLASS("UStruct", "UField"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("SuperField", "UStruct | None"),
                         &UStruct::SuperField)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Children", "UField | None"), &UStruct::Children)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertySize", "int"), &UStruct::PropertySize)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyLink", "UProperty | None"),
                         &UStruct::PropertyLink)
#if UNREALSDK_USTRUCT_HAS_ALIGNMENT
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("MinAlignment", "int"), &UStruct::MinAlignment)
#endif
#if UNREALSDK_PROPERTIES_ARE_FFIELD
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ChildProperties", "FField | None"),
                         &UStruct::MinAlignment)
#endif
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_fields", "Iterator[UField]"),
            [](UStruct* self) {
                auto fields = self->fields();
                // I don't like how calling this with the `fields` directly takes an lvalue
                // reference - would prefer to move an rvalue.
                // Testing, everything still works fine, there's no memory leak, but prefer to
                // manually call this with the iterators anyway
                return py::make_iterator(fields.begin(), fields.end());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Iterates over all fields in the struct.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    An iterator over all fields in the struct.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_properties", "Iterator[UProperty]"),
            [](UStruct* self) {
                auto props = self->properties();
                return py::make_iterator(props.begin(), props.end());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Iterates over all properties in the struct.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    An iterator over all properties in the struct.\n"))
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_superfields", "Iterator[UStruct]"),
            [](UStruct* self) {
                auto superfields = self->superfields();
                return py::make_iterator(superfields.begin(), superfields.end());
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Iterates over this struct and it's superfields.\n"
                                          "\n"
                                          "Note this includes this struct itself.\n"
                                          "\n"
                                          "Returns:\n"
                                          "    An iterator over all superfields in the struct.\n"))
        .def(PYUNREALSDK_STUBGEN_METHOD("_inherits", "bool"), &UStruct::inherits,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Checks if this structs inherits from another.\n"
                 "\n"
                 "Also returns true if this struct *is* the given struct.\n"
                 "\n"
                 "Args:\n"
                 "    base_struct: The base struct to check if this inherits from.\n"
                 "Returns:\n"
                 "    True if this struct is the given struct, or inherits from it.\n"),
             PYUNREALSDK_STUBGEN_ARG("base_struct"_a, "UStruct", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("_get_struct_size", "int"), &UStruct::get_struct_size,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Gets the actual size of the described structure, including alignment.\n"
                 "\n"
                 "Returns:\n"
                 "    The size which must be allocated.\n"))
        .def(
#ifdef UNREALSDK_PROPERTIES_ARE_FFIELD
            PYUNREALSDK_STUBGEN_METHOD("_find", "UField | UProperty"),
#else
            PYUNREALSDK_STUBGEN_METHOD("_find", "UField"),
#endif
            [](UStruct* self, FName name) {
                auto ret = self->find(name);

                // UFields are UObjects
                auto field = ret.as_uobject();
                if (field != nullptr) {
                    return py::cast(field);
                }

                // Properties might be FFields (if enabled)
                auto prop = ret.as_ffield();
                if (prop != nullptr) {
                    return py::cast(prop);
                }

                // This should be impossible, because the find class should throw first
                throw std::invalid_argument("Couldn't find field " + (std::string)name);
            },
            PYUNREALSDK_STUBGEN_DOCSTRING("Finds a child field by name.\n"
                                          "\n"
                                          "Throws an exception if the child is not found.\n"
                                          "\n"
                                          "Args:\n"
                                          "    name: The name of the child field.\n"
                                          "Returns:\n"
                                          "    The found child field.\n"),
            PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ))
        .def(PYUNREALSDK_STUBGEN_METHOD("_find_prop", "UProperty"), &UStruct::find_prop,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Finds a child property by name.\n"
                 "\n"
                 "When known to be a property, this is more efficient than _find.\n"
                 "\n"
                 "Throws an exception if the child is not found.\n"
                 "\n"
                 "Args:\n"
                 "    name: The name of the child property.\n"
                 "Returns:\n"
                 "    The found child property.\n"),
             PYUNREALSDK_STUBGEN_ARG("name"_a, "str", ));

    // ======== Third Layer Subclasses ========

    PyUEClass<FGbxDefPtrProperty, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("FGbxDefPtrProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Struct", "UScriptStruct"),
                         &FGbxDefPtrProperty::Struct);

    PyUEClass<UArrayProperty, UProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("UArrayProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Inner", "UProperty"), &UArrayProperty::Inner);

    PyUEClass<UBoolProperty, UProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("UBoolProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("FieldMask", "int"), &UBoolProperty::FieldMask);

    PyUEClass<UByteProperty, UProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("UByteProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Enum", "UEnum | None"), &UByteProperty::Enum);

    PyUEClass<UClass, UStruct>(mod, PYUNREALSDK_STUBGEN_CLASS("UClass", "UStruct"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ClassDefaultObject", "UObject"),
                         &UClass::ClassDefaultObject)
        .def(
            PYUNREALSDK_STUBGEN_METHOD("_implements", "bool"),
            [](UClass* self, UClass* interface) { return self->implements(interface, nullptr); },
            PYUNREALSDK_STUBGEN_DOCSTRING(
                "Checks if this class implements a given interface.\n"
                "\n"
                "Args:\n"
                "    interface: The interface to check.\n"
                "Returns:\n"
                "    True if this class implements the interface, false otherwise.\n"),
            PYUNREALSDK_STUBGEN_ARG("interface"_a, "UClass", ))
        .def_property_readonly(PYUNREALSDK_STUBGEN_READONLY_PROP("Interfaces", "list[UClass]"),
                               [](const UClass* self) {
                                   std::vector<UClass*> interfaces{};
                                   for (const auto& iface : self->Interfaces()) {
                                       interfaces.push_back(iface.Class);
                                   }
                                   return interfaces;
                               });

    PyUEClass<UDelegateProperty, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UDelegateProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Signature", "UFunction"),
                         &UDelegateProperty::Signature);

    PyUEClass<UDoubleProperty, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UDoubleProperty", "UProperty"));

    PyUEClass<UEnumProperty, UProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("UEnumProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("UnderlyingProp", "UProperty"),
                         &UEnumProperty::UnderlyingProp)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Enum", "UEnum"), &UEnumProperty::Enum);

    PyUEClass<UFloatProperty, UProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("UFloatProperty", "UProperty"));

    PyUEClass<UFunction, UStruct>(mod, PYUNREALSDK_STUBGEN_CLASS("UFunction", "UStruct"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("FunctionFlags", "int"),
                         &UFunction::FunctionFlags)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("NumParams", "int"), &UFunction::NumParams)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ParamsSize", "int"), &UFunction::ParamsSize)
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ReturnValueOffset", "int"),
                         &UFunction::ReturnValueOffset)
        .def(PYUNREALSDK_STUBGEN_METHOD("_find_return_param", "UProperty"),
             &UFunction::find_return_param,
             PYUNREALSDK_STUBGEN_DOCSTRING(
                 "Finds the return param for this function (if it exists).\n"
                 "\n"
                 "Returns:\n"
                 "    The return param, or None if it doesn't exist.\n"));

    PyUEClass<UInt8Property, UProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("UInt8Property", "UProperty"));

    PyUEClass<UInt16Property, UProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("UInt16Property", "UProperty"));

    PyUEClass<UInt64Property, UProperty>(mod,
                                         PYUNREALSDK_STUBGEN_CLASS("UInt64Property", "UProperty"));

    PyUEClass<UInterfaceProperty, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UInterfaceProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("InterfaceClass", "UClass"),
                         &UInterfaceProperty::InterfaceClass);

    PyUEClass<UIntProperty, UProperty>(mod, PYUNREALSDK_STUBGEN_CLASS("UIntProperty", "UProperty"));

    PyUEClass<UMulticastDelegateProperty, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UMulticastDelegateProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Signature", "UFunction"),
                         &UMulticastDelegateProperty::Signature);

    PyUEClass<UNameProperty, UProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("UNameProperty", "UProperty"));

    PyUEClass<UObjectProperty, UProperty>(mod,
                                          PYUNREALSDK_STUBGEN_CLASS("UObjectProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("PropertyClass", "UClass"),
                         &UObjectProperty::PropertyClass);

    PyUEClass<UScriptStruct, UStruct>(mod, PYUNREALSDK_STUBGEN_CLASS("UScriptStruct", "UStruct"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("StructFlags", "int"),
                         &UScriptStruct::StructFlags);

    PyUEClass<UStrProperty, UProperty>(mod, PYUNREALSDK_STUBGEN_CLASS("UStrProperty", "UProperty"));

    PyUEClass<UStructProperty, UProperty>(mod,
                                          PYUNREALSDK_STUBGEN_CLASS("UStructProperty", "UProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("Struct", "UScriptStruct"),
                         &UStructProperty::Struct);

    PyUEClass<UTextProperty, UProperty>(mod,
                                        PYUNREALSDK_STUBGEN_CLASS("UTextProperty", "UProperty"));

    PyUEClass<UUInt16Property, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UUInt16Property", "UProperty"));

    PyUEClass<UUInt32Property, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UUInt32Property", "UProperty"));

    PyUEClass<UUInt64Property, UProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UUInt64Property", "UProperty"));

    // ======== Fourth Layer Subclasses ========

    PyUEClass<UBlueprintGeneratedClass, UClass>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UBlueprintGeneratedClass", "UClass"));

    PyUEClass<UByteAttributeProperty, UByteProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UByteAttributeProperty", "UByteProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ModifierStackProperty", "UArrayProperty"),
                         &UByteAttributeProperty::ModifierStackProperty)
        .def_member_prop(
            PYUNREALSDK_STUBGEN_ATTR("OtherAttributeProperty", "UByteAttributeProperty"),
            &UByteAttributeProperty::OtherAttributeProperty);

    PyUEClass<UClassProperty, UObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UClassProperty", "UObjectProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("MetaClass", "UClass"),
                         &UClassProperty::MetaClass);

    PyUEClass<UComponentProperty, UObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UComponentProperty", "UObjectProperty"));

    PyUEClass<UFloatAttributeProperty, UFloatProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UFloatAttributeProperty", "UFloatProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ModifierStackProperty", "UArrayProperty"),
                         &UFloatAttributeProperty::ModifierStackProperty)
        .def_member_prop(
            PYUNREALSDK_STUBGEN_ATTR("OtherAttributeProperty", "UFloatAttributeProperty"),
            &UFloatAttributeProperty::OtherAttributeProperty);

    PyUEClass<UIntAttributeProperty, UIntProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UIntAttributeProperty", "UIntProperty"))
        .def_member_prop(PYUNREALSDK_STUBGEN_ATTR("ModifierStackProperty", "UArrayProperty"),
                         &UIntAttributeProperty::ModifierStackProperty)
        .def_member_prop(
            PYUNREALSDK_STUBGEN_ATTR("OtherAttributeProperty", "UIntAttributeProperty"),
            &UIntAttributeProperty::OtherAttributeProperty);

    // ULazyObjectProperty - registered elsewhere
    // USoftObjectProperty - registered elsewhere

    PyUEClass<UWeakObjectProperty, UObjectProperty>(
        mod, PYUNREALSDK_STUBGEN_CLASS("UWeakObjectProperty", "UObjectProperty"));

    // ======== Fifth Layer Subclasses ========

    // USoftClassProperty - registered elsewhere
}

}  // namespace pyunrealsdk::unreal

#endif
