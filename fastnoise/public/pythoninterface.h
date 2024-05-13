///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "technique.h"

namespace fastnoise
{
    inline PyObject* FilterTypeToString(PyObject* self, PyObject* args)
    {
        int value;
        if (!PyArg_ParseTuple(args, "i:FilterTypeToString", &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        switch((FilterType)value)
        {
            case FilterType::Box: return Py_BuildValue("s", "Box");
            case FilterType::Gaussian: return Py_BuildValue("s", "Gaussian");
            case FilterType::Binomial: return Py_BuildValue("s", "Binomial");
            case FilterType::Exponential: return Py_BuildValue("s", "Exponential");
            case FilterType::WeightedExponential: return Py_BuildValue("s", "WeightedExponential");
            default: return Py_BuildValue("s", "<invalid FilterType value>");
        }
    }

    inline PyObject* SampleSpaceToString(PyObject* self, PyObject* args)
    {
        int value;
        if (!PyArg_ParseTuple(args, "i:SampleSpaceToString", &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        switch((SampleSpace)value)
        {
            case SampleSpace::Real: return Py_BuildValue("s", "Real");
            case SampleSpace::Circle: return Py_BuildValue("s", "Circle");
            case SampleSpace::Vector2: return Py_BuildValue("s", "Vector2");
            case SampleSpace::Vector3: return Py_BuildValue("s", "Vector3");
            case SampleSpace::Vector4: return Py_BuildValue("s", "Vector4");
            case SampleSpace::Sphere: return Py_BuildValue("s", "Sphere");
            default: return Py_BuildValue("s", "<invalid SampleSpace value>");
        }
    }

    inline PyObject* SampleDistributionToString(PyObject* self, PyObject* args)
    {
        int value;
        if (!PyArg_ParseTuple(args, "i:SampleDistributionToString", &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        switch((SampleDistribution)value)
        {
            case SampleDistribution::Uniform1D: return Py_BuildValue("s", "Uniform1D");
            case SampleDistribution::Gauss1D: return Py_BuildValue("s", "Gauss1D");
            case SampleDistribution::Tent1D: return Py_BuildValue("s", "Tent1D");
            case SampleDistribution::Uniform2D: return Py_BuildValue("s", "Uniform2D");
            case SampleDistribution::Uniform3D: return Py_BuildValue("s", "Uniform3D");
            case SampleDistribution::Uniform4D: return Py_BuildValue("s", "Uniform4D");
            case SampleDistribution::UniformSphere: return Py_BuildValue("s", "UniformSphere");
            case SampleDistribution::UniformHemisphere: return Py_BuildValue("s", "UniformHemisphere");
            case SampleDistribution::CosineHemisphere: return Py_BuildValue("s", "CosineHemisphere");
            default: return Py_BuildValue("s", "<invalid SampleDistribution value>");
        }
    }

    inline PyObject* Set_filterXparams(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float4 value;

        if (!PyArg_ParseTuple(args, "iffff:Set_filterXparams", &contextIndex, &value[0], &value[1], &value[2], &value[3]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_filterXparams = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_filterYparams(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float4 value;

        if (!PyArg_ParseTuple(args, "iffff:Set_filterYparams", &contextIndex, &value[0], &value[1], &value[2], &value[3]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_filterYparams = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_filterZparams(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float4 value;

        if (!PyArg_ParseTuple(args, "iffff:Set_filterZparams", &contextIndex, &value[0], &value[1], &value[2], &value[3]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_filterZparams = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_separate(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_separate", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_separate = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_separateWeight(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float value;

        if (!PyArg_ParseTuple(args, "if:Set_separateWeight", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_separateWeight = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_sampleSpace(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_sampleSpace", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_sampleSpace = (SampleSpace)value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_sampleDistribution(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_sampleDistribution", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_sampleDistribution = (SampleDistribution)value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"FilterTypeToString", FilterTypeToString, METH_VARARGS, ""},
        {"SampleSpaceToString", SampleSpaceToString, METH_VARARGS, ""},
        {"SampleDistributionToString", SampleDistributionToString, METH_VARARGS, ""},
        {"Set_filterXparams", Set_filterXparams, METH_VARARGS, ""},
        {"Set_filterYparams", Set_filterYparams, METH_VARARGS, ""},
        {"Set_filterZparams", Set_filterZparams, METH_VARARGS, ""},
        {"Set_separate", Set_separate, METH_VARARGS, "Whether to use "separate" mode, which makes STBN-style samples"},
        {"Set_separateWeight", Set_separateWeight, METH_VARARGS, "If "separate" is true, the weight for blending between temporal and spatial filter"},
        {"Set_sampleSpace", Set_sampleSpace, METH_VARARGS, ""},
        {"Set_sampleDistribution", Set_sampleDistribution, METH_VARARGS, ""},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "fastnoise", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        PyModule_AddIntConstant(module, "FilterType_Box", 0);
        PyModule_AddIntConstant(module, "FilterType_Gaussian", 1);
        PyModule_AddIntConstant(module, "FilterType_Binomial", 2);
        PyModule_AddIntConstant(module, "FilterType_Exponential", 3);
        PyModule_AddIntConstant(module, "FilterType_WeightedExponential", 4);
        PyModule_AddIntConstant(module, "SampleSpace_Real", 0);
        PyModule_AddIntConstant(module, "SampleSpace_Circle", 1);
        PyModule_AddIntConstant(module, "SampleSpace_Vector2", 2);
        PyModule_AddIntConstant(module, "SampleSpace_Vector3", 3);
        PyModule_AddIntConstant(module, "SampleSpace_Vector4", 4);
        PyModule_AddIntConstant(module, "SampleSpace_Sphere", 5);
        PyModule_AddIntConstant(module, "SampleDistribution_Uniform1D", 0);
        PyModule_AddIntConstant(module, "SampleDistribution_Gauss1D", 1);
        PyModule_AddIntConstant(module, "SampleDistribution_Tent1D", 2);
        PyModule_AddIntConstant(module, "SampleDistribution_Uniform2D", 3);
        PyModule_AddIntConstant(module, "SampleDistribution_Uniform3D", 4);
        PyModule_AddIntConstant(module, "SampleDistribution_Uniform4D", 5);
        PyModule_AddIntConstant(module, "SampleDistribution_UniformSphere", 6);
        PyModule_AddIntConstant(module, "SampleDistribution_UniformHemisphere", 7);
        PyModule_AddIntConstant(module, "SampleDistribution_CosineHemisphere", 8);
        return module;
    }
};
