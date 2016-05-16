#include "pch_bcl.h"
#include "Pointer.h"

using namespace Causality;

Pointer::Pointer() : m_parent(nullptr), m_buttonCount(1), m_buttonState(0), m_type(PointerType_Unknown) {}

Pointer::~Pointer() = default;
