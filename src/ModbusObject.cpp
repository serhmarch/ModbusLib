#include "ModbusObject.h"
#include "ModbusObject_p.h"

ModbusObject::ModbusObject() :
    ModbusObject(new ModbusObjectPrivate)
{
}

ModbusObject::ModbusObject(ModbusObjectPrivate *o) :
    d_ptr(o)
{
}

ModbusObject::~ModbusObject()
{
    for (ModbusObjectPrivate::const_iterator it = d_ptr->begin(); it != d_ptr->end(); it++)
    {
        for (void *ptr : it->second)
        {
            delete reinterpret_cast<ModbusSlotBase<void>*>(ptr);
        }
    }
    delete d_ptr;
}

void *ModbusObject::slot(void *signalMethodPtr, int i) const
{
    ModbusObjectPrivate::const_iterator it = d_ptr->find(signalMethodPtr);
    if (it != d_ptr->end())
    {
        std::list<void*>::const_iterator lit = it->second.begin();
        std::advance(lit, i);
        if (lit != it->second.end())
            return *lit;
    }
    return nullptr;
}

void ModbusObject::setSlot(void *signalMethodPtr, void *slotPtr)
{
    (*d_ptr)[signalMethodPtr].push_back(slotPtr);
}

void ModbusObject::disconnect(void *object, void *methodOrFunc)
{
    for (ModbusObjectPrivate::iterator it = d_ptr->begin(); it != d_ptr->end(); )
    {
        for (std::list<void*>::iterator i = it->second.begin(); i != it->second.end(); )
        {
            ModbusSlotBase<void> *callback = reinterpret_cast<ModbusSlotBase<void>*>(*i);
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
            it = d_ptr->erase(it);
            continue;
        }
        it++;
    }
}
