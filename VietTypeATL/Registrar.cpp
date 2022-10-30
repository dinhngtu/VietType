#include "Registrar.h"
#include "Register.h"

namespace VietType {

STDMETHODIMP CVietTypeRegistrar::ActivateProfiles() {
    return ::ActivateProfiles();
}

STDMETHODIMP CVietTypeRegistrar::DeactivateProfiles() {
    return ::DeactivateProfiles();
}

STDMETHODIMP CVietTypeRegistrar::IsProfileActivated() {
    return ::IsProfileActivated();
}

} // namespace VietType
