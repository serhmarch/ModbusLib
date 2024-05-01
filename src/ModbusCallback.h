/*!
 * \file   ModbusCallback.h
 * \brief The header file defines the class templates used to create signal callbacks
 *
 * \author march
 * \date   April 2024
 *********************************************************************/
#ifndef MODBUS_CALLBACK_H
#define MODBUS_CALLBACK_H

#include <list>
#include <unordered_map>

#include "Modbus.h"

namespace Modbus {


template <class T, class ReturnType, class ... Args>
using MethodPointer = ReturnType(T::*)(Args...);

template <class ReturnType, class ... Args>
using FunctionPointer = ReturnType (*)(Args...);

template <class ReturnType, class ... Args>
class CallbackBase
{
public:
    virtual ~CallbackBase() {}
    virtual void *object() const { return nullptr; }
    virtual void *methodOrFunction() const = 0;
    virtual ReturnType exec(Args ...  args) = 0;
};



template <class T, class ReturnType, class ... Args>
class CallbackMethod : public CallbackBase<ReturnType, Args ...>
{
public:
    CallbackMethod(T* object, MethodPointer<T, ReturnType, Args...> methodPtr) : m_object(object), m_methodPtr(methodPtr) {}

public:
    void *object() const override { return m_object; }
    void *methodOrFunction() const override { return reinterpret_cast<void*>(m_voidPtr); }

    ReturnType exec(Args ...  args) override
    {
        return (m_object->*m_methodPtr)(args...);
    }

private:
    T* m_object;
    union
    {
        MethodPointer<T, ReturnType, Args...> m_methodPtr;
        void *m_voidPtr;
    };
};


template <class ReturnType, class ... Args>
class CallbackFunction : public CallbackBase<ReturnType, Args ...>
{
public:
    CallbackFunction(FunctionPointer<ReturnType, Args...> funcPtr) : m_funcPtr(funcPtr) {}

public:
    void *methodOrFunction() const override { return m_funcPtr; }
    ReturnType exec(Args ...  args) override
    {
        return m_funcPtr(args...);
    }

private:
    FunctionPointer<ReturnType, Args...> m_funcPtr;
};

class MODBUS_EXPORT CallbackSignals
{
private:
    typedef std::unordered_map<void*, std::list<void*> > Callbacks;
    Callbacks m_callbacks;

    void disconnect(void *object, void *methodOrFunc)
    {
        for (Callbacks::iterator it = m_callbacks.begin(); it != m_callbacks.end(); )
        {
            for (std::list<void*>::iterator i = it->second.begin(); i != it->second.end(); )
            {
                CallbackBase<void> *callback = reinterpret_cast<CallbackBase<void>*>(*i);
                bool del = false;
                if (object)
                {
                    del = ((callback->object() == object) && ((methodOrFunc == nullptr) || (callback->methodOrFunction() == methodOrFunc)));
                }
                else if (callback->methodOrFunction() == methodOrFunc)
                {
                    del = true;
                }
                if (del)
                {
                    i = it->second.erase(i);
                    delete callback;
                    continue;
                }
                i++;
            }
            if (it->second.size() == 0)
            {
                it = m_callbacks.erase(it);
                continue;
            }
            it++;
        }
    }

protected:
    template <class T, class ... Args>
    void emitSignal(MethodPointer<T, void, Args ...> thisMethod, Args ... args)
    {
        union {
            MethodPointer<T, void, Args ...> thisMethod;
            void* voidPtr;
        } converter;
        converter.thisMethod = thisMethod;

        void *ptr = reinterpret_cast<void*>(converter.voidPtr);
        Callbacks::const_iterator it = m_callbacks.find(ptr);
        if (it != m_callbacks.end())
        {
            for (void* item : it->second)
            {
                CallbackBase<void, Args...> *callback = reinterpret_cast<CallbackBase<void, Args...> *>(item);
                callback->exec(args...);
            }
        }
    }

public:
    ~CallbackSignals()
    {
        for (Callbacks::const_iterator it = m_callbacks.begin(); it != m_callbacks.end(); it++)
        {
            for (void *ptr : it->second)
            {
                delete reinterpret_cast<CallbackBase<void>*>(ptr);
            }
        }
    }

    template <class This, class T, class ReturnType, class ... Args>
    void connect(MethodPointer<This, ReturnType, Args ...> thisMethod, T *object, MethodPointer<T, ReturnType, Args ...> objectMethodPtr)
    {
        CallbackMethod<T, ReturnType, Args ...> *callback = new CallbackMethod<T, ReturnType, Args ...>(object, objectMethodPtr);
        union {
            MethodPointer<This, ReturnType, Args ...> thisMethod;
            void* voidPtr;
        } converter;
        converter.thisMethod = thisMethod;

        m_callbacks[converter.voidPtr].push_back(callback);
    }

    template <class This, class ReturnType, class ... Args>
    void connect(MethodPointer<This, ReturnType, Args ...> thisMethod, FunctionPointer<ReturnType, Args ...> funcPtr)
    {
        CallbackFunction<ReturnType, Args ...> *callback = new CallbackFunction<ReturnType, Args ...>(funcPtr);
        union {
            MethodPointer<This, ReturnType, Args ...> thisMethod;
            void* voidPtr;
        } converter;
        converter.thisMethod = thisMethod;

        m_callbacks[converter.voidPtr].push_back(callback);
    }

    template <class ReturnType, class ... Args>
    inline void disconnect(FunctionPointer<ReturnType, Args ...> funcPtr)
    {
        disconnect(nullptr, funcPtr);
    }

    template <class T, class ReturnType, class ... Args>
    inline void disconnect(T *object, MethodPointer<T, ReturnType, Args ...> objectMethodPtr)
    {
        union {
            MethodPointer<T, ReturnType, Args ...> objectMethodPtr;
            void* voidPtr;
        } converter;
        converter.objectMethodPtr = objectMethodPtr;
        disconnect(object, converter.voidPtr);
    }

    template <class T>
    inline void disconnect(T *object)
    {
        disconnect(object, nullptr);
    }
};


} // namespace Modbus

#endif // MODBUS_CALLBACK_H
