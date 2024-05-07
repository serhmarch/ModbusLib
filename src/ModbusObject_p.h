#ifndef MODBUSOBJECT_P_H
#define MODBUSOBJECT_P_H

#include <list>
#include <unordered_map>

#include "Modbus.h"

using namespace Modbus;

class ModbusObjectPrivate : public std::unordered_map<void*, std::list<void*> >
{
public:
    virtual ~ModbusObjectPrivate()
    {
    }

public:
    using std::unordered_map<void*, std::list<void*> >::unordered_map;
};

#endif // MODBUSOBJECT_P_H
