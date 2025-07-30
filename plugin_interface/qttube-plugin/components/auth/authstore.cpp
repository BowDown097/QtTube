#include "authstore.h"

namespace QtTubePlugin
{
    AuthUser* AuthStoreBase::activeBaseLogin() const
    {
        auto it = std::ranges::find_if(m_credentials, &AuthUser::active);
        return it != m_credentials.end() ? it->get() : nullptr;
    }

    const QList<AuthUser*> AuthStoreBase::baseCredentials() const
    {
        QList<AuthUser*> out;
        out.reserve(m_credentials.size());
        std::ranges::transform(m_credentials, std::back_inserter(out), &std::unique_ptr<AuthUser>::get);
        return out;
    }

    void AuthStoreBase::clear()
    {
        ConfigStore::clear();
        m_credentials.clear();
        unauthenticate();
    }

    bool AuthStoreBase::isEmpty() const
    {
        return m_credentials.empty();
    }

    qsizetype AuthStoreBase::size() const
    {
        return m_credentials.size();
    }
}
