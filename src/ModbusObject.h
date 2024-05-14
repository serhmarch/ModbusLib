/*!
 * \file   ModbusObject.h
 * \brief  The header file defines the class templates used to create signal/slot-like mechanism
 *
 * \author march
 * \date   May 2024
 *********************************************************************/
#ifndef MODBUSOBJECT_H
#define MODBUSOBJECT_H

#include "Modbus.h"


template <class T, class ReturnType, class ... Args>
using ModbusMethodPointer = ReturnType(T::*)(Args...);

template <class ReturnType, class ... Args>
using ModbusFunctionPointer = ReturnType (*)(Args...);

template <class ReturnType, class ... Args>
class ModbusSlotBase
{
public:
    virtual ~ModbusSlotBase() {}
    virtual void *object() const { return nullptr; }
    virtual void *methodOrFunction() const = 0;
    virtual ReturnType exec(Args ...  args) = 0;
};



template <class T, class ReturnType, class ... Args>
class ModbusSlotMethod : public ModbusSlotBase<ReturnType, Args ...>
{
public:
    ModbusSlotMethod(T* object, ModbusMethodPointer<T, ReturnType, Args...> methodPtr) : m_object(object), m_methodPtr(methodPtr) {}

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
        ModbusMethodPointer<T, ReturnType, Args...> m_methodPtr;
        void *m_voidPtr;
    };
};


template <class ReturnType, class ... Args>
class ModbusSlotFunction : public ModbusSlotBase<ReturnType, Args ...>
{
public:
    ModbusSlotFunction(ModbusFunctionPointer<ReturnType, Args...> funcPtr) : m_funcPtr(funcPtr) {}

public:
    void *methodOrFunction() const override { return m_voidPtr; }
    ReturnType exec(Args ...  args) override
    {
        return m_funcPtr(args...);
    }

private:
    union
    {
        ModbusFunctionPointer<ReturnType, Args...> m_funcPtr;
        void *m_voidPtr;
    };
};

class ModbusObjectPrivate;

class MODBUS_EXPORT ModbusObject
{
public:
    static ModbusObject *sender();

public:
    ModbusObject();
    virtual ~ModbusObject();

public:
    const Modbus::Char *objectName() const;
    void setObjectName(const Modbus::Char *name);

public:
    template <class SignalClass, class T, class ReturnType, class ... Args>
    void connect(ModbusMethodPointer<SignalClass, ReturnType, Args ...> signalMethodPtr, T *object, ModbusMethodPointer<T, ReturnType, Args ...> objectMethodPtr)
    {
        ModbusSlotMethod<T, ReturnType, Args ...> *slotMethod = new ModbusSlotMethod<T, ReturnType, Args ...>(object, objectMethodPtr);
        union {
            ModbusMethodPointer<SignalClass, ReturnType, Args ...> signalMethodPtr;
            void* voidPtr;
        } converter;
        converter.signalMethodPtr = signalMethodPtr;
        setSlot(converter.voidPtr, slotMethod);
    }

    template <class SignalClass, class ReturnType, class ... Args>
    void connect(ModbusMethodPointer<SignalClass, ReturnType, Args ...> signalMethodPtr, ModbusFunctionPointer<ReturnType, Args ...> funcPtr)
    {
        ModbusSlotFunction<ReturnType, Args ...> *slotFunc = new ModbusSlotFunction<ReturnType, Args ...>(funcPtr);
        union {
            ModbusMethodPointer<SignalClass, ReturnType, Args ...> signalMethodPtr;
            void* voidPtr;
        } converter;
        converter.signalMethodPtr = signalMethodPtr;
        setSlot(converter.voidPtr, slotFunc);
    }

    template <class ReturnType, class ... Args>
    inline void disconnect(ModbusFunctionPointer<ReturnType, Args ...> funcPtr)
    {
        disconnect(nullptr, funcPtr);
    }

    template <class T, class ReturnType, class ... Args>
    inline void disconnect(T *object, ModbusMethodPointer<T, ReturnType, Args ...> objectMethodPtr)
    {
        union {
            ModbusMethodPointer<T, ReturnType, Args ...> objectMethodPtr;
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

    template <class T, class ... Args>
    void emitSignal(ModbusMethodPointer<T, void, Args ...> thisMethod, Args ... args)
    {
        union {
            ModbusMethodPointer<T, void, Args ...> thisMethod;
            void* voidPtr;
        } converter;
        converter.thisMethod = thisMethod;

        pushSender(this);
        int i = 0;
        while (void* itemSlot = slot(converter.voidPtr, i++))
        {
            ModbusSlotBase<void, Args...> *slotBase = reinterpret_cast<ModbusSlotBase<void, Args...> *>(itemSlot);
            slotBase->exec(args...);
        }
        popSender();
    }

private:
    void *slot(void *signalMethodPtr, int i) const;
    void setSlot(void *signalMethodPtr, void *slotPtr);
    void disconnect(void *object, void *methodOrFunc);

private:
    static void pushSender(ModbusObject *sender);
    static void popSender();

protected:
    ModbusObjectPrivate *d_ptr;
    ModbusObject(ModbusObjectPrivate *d);
};


#endif // MODBUSOBJECT_H
