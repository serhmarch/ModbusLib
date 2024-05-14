#ifndef MODBUSOBJECT_P_H
#define MODBUSOBJECT_P_H

#include <list>
#include <unordered_map>

#include "Modbus.h"

using namespace Modbus;

class ModbusObjectPrivate
{
public:
    typedef std::list<void*> SignalSlots;
    typedef std::unordered_map<void*, SignalSlots> Signals;
    using const_iterator = Signals::const_iterator;
    using iterator = Signals::iterator;

public:
    virtual ~ModbusObjectPrivate()
    {
    }

public:
    inline Signals::const_iterator begin() const { return signals.begin(); }
    inline Signals::const_iterator end() const { return signals.end(); }
    inline Signals::const_iterator find(void *signal) const { return signals.find(signal); }

    inline Signals::const_iterator cbegin() const { return signals.cbegin(); }
    inline Signals::const_iterator cend() const { return signals.cend(); }

    inline Signals::iterator begin() { return signals.begin(); }
    inline Signals::iterator end() { return signals.end(); }
    inline Signals::iterator find(void *&signal) { return signals.find(signal); }

    inline iterator erase( iterator pos ) { return signals.erase(pos); }
    inline iterator erase( const_iterator pos ) { return signals.erase(pos); }
    inline iterator erase( const_iterator first, const_iterator last ) { return signals.erase(first, last); }

    inline SignalSlots& operator[]( void* &signal ) { return signals[signal]; }
    inline SignalSlots& operator[]( void *&& signal ) { return signals[signal]; }

public:
    String objectName;
    Signals signals;
};

#endif // MODBUSOBJECT_P_H
