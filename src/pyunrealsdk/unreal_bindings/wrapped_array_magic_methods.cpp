#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/wrapped_array.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/wrappers/wrapped_array.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal::impl {

void array_py_new(const py::args& /*args*/, const py::kwargs& /*kwargs*/) {
    throw py::type_error("Cannot create new instances of wrapped arrays.");
}

WrappedArray* array_py_init(const py::args& /*args*/, const py::kwargs& /*kwargs*/) {
    throw py::type_error("Cannot create new instances of wrapped arrays.");
}

std::string array_py_repr(const WrappedArray& self) {
    std::ostringstream output;
    output << "[";

    for (size_t i = 0; i < self.size(); i++) {
        if (i > 0) {
            output << ", ";
        }

        output << py::repr(array_get(self, i));
    }

    output << "]";
    return output.str();
}

py::object array_py_getitem(const WrappedArray& self, py::ssize_t py_idx) {
    return array_get(self, convert_py_idx(self, py_idx));
}

py::list array_py_getitem_slice(const WrappedArray& self, const py::slice& slice) {
    py::ssize_t start = 0;
    py::ssize_t stop = 0;
    py::ssize_t step = 0;
    py::ssize_t slicelength = 0;
    if (!slice.compute(static_cast<py::ssize_t>(self.size()), &start, &stop, &step, &slicelength)) {
        throw py::error_already_set();
    }

    py::list ret{slicelength};
    for (auto i = 0; i < slicelength; i++) {
        ret[i] = array_get(self, start);
        start += step;
    }
    return ret;
}

void array_py_setitem(WrappedArray& self, py::ssize_t py_idx, const py::object& value) {
    array_set(self, convert_py_idx(self, py_idx), value);
}

void array_py_setitem_slice(WrappedArray& self, const py::slice& slice, const py::sequence& value) {
    auto values_size = static_cast<py::ssize_t>(value.size());

    py::ssize_t start = 0;
    py::ssize_t stop = 0;
    py::ssize_t step = 0;
    py::ssize_t slicelength = 0;
    if (!slice.compute(static_cast<py::ssize_t>(self.size()), &start, &stop, &step, &slicelength)) {
        throw py::error_already_set();
    }

    // If we have 1-1 replacements
    if (slicelength == values_size) {
        // Allow arbitrary steps
        for (auto i = 0; i < slicelength; i++) {
            array_set(self, start, value[i]);
            start += step;
        }
        return;
    }

    // Otherwise, if sizes don't match, we must not be doing an extended slice, it needs to be
    // continuous
    // This logic sounds backwards, but it lets our simpler code early exit, and it works the same
    // way as list
    if (step != 1 && step != -1) {
        throw py::value_error(
            std::format("attempt to assign sequence of size {} to extended slice of size {}",
                        value.size(), slicelength));
    }

    if (step < 0) {
        auto tmp = start;
        start = stop;
        stop = tmp;
        step *= -1;
    }

    auto new_size = self.size() + values_size - slicelength;
    if (new_size > self.capacity()) {
        self.reserve(new_size);
    }

    // As much as I'd love to memmove the data buffer here, we can't because we don't know that the
    // values in our sequence are valid types.
    // Instead we need to stick to the basic operations, such that an exception at any point leaves
    // the array in a valid state (even if it's not what was intended).
    // Choosing to do this by deleting all overwritten objects, then inserting all the new ones.
    array_py_delitem_slice(self, py::slice(start, stop, 1));

    for (auto value_idx = 0; value_idx < values_size; value_idx++) {
        array_py_insert(self, start + value_idx, value[value_idx]);
    }
}

void array_py_delitem(WrappedArray& self, py::ssize_t py_idx) {
    auto start = convert_py_idx(self, py_idx);
    array_delete_range(self, start, start + 1);
}
void array_py_delitem_slice(WrappedArray& self, const py::slice& slice) {
    py::ssize_t start = 0;
    py::ssize_t stop = 0;
    py::ssize_t step = 0;
    py::ssize_t slicelength = 0;

    if (!slice.compute(static_cast<py::ssize_t>(self.size()), &start, &stop, &step, &slicelength)) {
        throw py::error_already_set();
    }

    // If nothing to delete, can early exit
    if (slicelength == 0) {
        return;
    }

    // If we don't have continuous ranges
    if (step != 1 && step != -1) {
        // Delete each index individually
        for (auto i = 0; i < slicelength; i++) {
            array_delete_range(self, start, start + 1);
            // Subtract one for the entry we just deleted
            start += step - 1;
        }
    } else {
        // Otherwise, we can delete everything in one go
        if (step < 0) {
            auto tmp = start;
            start = stop;
            stop = tmp;
        }
        array_delete_range(self, start, stop);
    }
}

py::iterator array_py_iter(const WrappedArray& self) {
    return py::make_iterator(ArrayIterator::begin(self), ArrayIterator::end(self));
}

py::iterator array_py_reversed(const WrappedArray& self) {
    return py::make_iterator(std::make_reverse_iterator(ArrayIterator::end(self)),
                             std::make_reverse_iterator(ArrayIterator::begin(self)));
}

bool array_py_contains(const WrappedArray& self, const py::object& value) {
    return std::find_if(ArrayIterator::begin(self), ArrayIterator::end(self),
                        [&value](const auto& other) { return value.equal(other); })
           != ArrayIterator::end(self);
}

py::list array_py_add(WrappedArray& self, const py::sequence& other) {
    return array_py_copy(self) + other;
}

py::list array_py_radd(WrappedArray& self, const py::sequence& other) {
    return other + array_py_copy(self);
}

WrappedArray& array_py_iadd(WrappedArray& self, const py::sequence& other) {
    array_py_extend(self, other);
    return self;
};

py::list array_py_mul(WrappedArray& self, py::ssize_t other) {
    if (other == 1) {
        return array_py_copy(self);
    }
    if (other < 1) {
        return py::list{};
    }

    auto template_list = array_py_copy(self);
    py::list ret_list{template_list};
    for (auto i = 1; i < other; i++) {
        ret_list += template_list;
    }

    return ret_list;
}

WrappedArray& array_py_imul(WrappedArray& self, py::ssize_t other) {
    if (other == 1) {
        return self;
    }
    if (other < 1) {
        array_py_clear(self);
        return self;
    }

    cast(self.type, [&self, other]<typename T>(const T* /*prop*/) {
        auto size = self.size();
        self.reserve(size * other);

        // Loop such that we're always adding elements to the end of the array
        // If we somehow error in the middle of the doing this, it'll leave us in a
        // stable state.
        for (auto repetition = 0; repetition < other; repetition++) {
            for (size_t base_idx = 0; base_idx < size; base_idx++) {
                auto copy_idx = (repetition * size) + base_idx;

                try {
                    self.set_at<T>(copy_idx, self.get_at<T>(base_idx));
                } catch (...) {
                    // On exception, make sure to delete whatever value may have just been set
                    self.destroy_at<T>(copy_idx);
                    throw;
                }

                self.resize(copy_idx + 1);
            }
        }
    });

    return self;
}

py::type array_py_class_getitem(const py::type& cls,
                                const py::args& /*args*/,
                                const py::kwargs& /*kwargs*/) {
    return cls;
}

}  // namespace pyunrealsdk::unreal::impl

#endif
