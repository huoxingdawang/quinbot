#pragma once

#include <string>

#include <Python.h>

namespace quinbot
{
namespace util
{
    inline int32_t pydict_as_int32( PyObject *dict, const char *key_str )
    {
        PyObject *key_s = PyUnicode_FromString(key_str);
        return PyLong_AsLong(PyDict_GetItem(dict, key_s));
    }

    inline int64_t pydict_as_int64( PyObject *dict, const char *key_str )
    {
        PyObject *key_s = PyUnicode_FromString(key_str);
        return PyLong_AsLongLong(PyDict_GetItem(dict, key_s));
    }

    inline std::string pydict_as_string( PyObject *dict, const char *key_str )
    {
        PyObject *key_s = PyUnicode_FromString(key_str);
        return std::string(PyUnicode_AsUTF8(PyDict_GetItem(dict, key_s)));
    }

    class PythonThreadLock
    {
    public:
        PythonThreadLock()
            :   state(PyGILState_Ensure())
        {}

        ~PythonThreadLock()
        {
            PyGILState_Release(state);
        }

    private:
        PyGILState_STATE state;
    };
}
}