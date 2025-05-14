#pragma once
#include <any>
#include <QException>
#include <wobjectimpl.h>

namespace QtTube
{
    class PluginException : public QException
    {
    public:
        enum class Severity { Normal, Minor };
        explicit PluginException(const QString& message, Severity severity = Severity::Normal)
            : m_message(message), m_severity(severity) {}
        explicit PluginException(QString&& message, Severity severity = Severity::Normal)
            : m_message(std::move(message)), m_severity(severity) {}
        const QString& message() const { return m_message; }
        Severity severity() const { return m_severity; }
    private:
        QString m_message;
        Severity m_severity;
    };

    template<typename T>
    class PluginReply : public QObject
    {
        W_OBJECT(PluginReply)
    public:
        std::any continuationData;

        void exception(const QtTube::PluginException& ex) W_SIGNAL(exception, ex)
        void finished(const T& object) W_SIGNAL(finished, object)

        // creates a reply that is automatically deleted when finished. you'll want to use this with multithreaded code.
        static PluginReply<T>* create()
        {
            PluginReply<T>* reply = new PluginReply<T>;
            connect(reply, &PluginReply<T>::exception, reply, &QObject::deleteLater);
            connect(reply, &PluginReply<T>::finished, reply, &QObject::deleteLater);
            return reply;
        }
    };

    template<>
    class PluginReply<void> : public QObject
    {
        W_OBJECT(PluginReply<void>)
    public:
        std::any continuationData;

        void exception(const QtTube::PluginException& ex) W_SIGNAL(exception, ex)
        void finished() W_SIGNAL(finished)

        // creates a reply that is automatically deleted when finished. you'll want to use this with multithreaded code.
        static PluginReply<void>* create()
        {
            PluginReply<void>* reply = new PluginReply<void>;
            connect(reply, &PluginReply<void>::exception, reply, &QObject::deleteLater);
            connect(reply, &PluginReply<void>::finished, reply, &QObject::deleteLater);
            return reply;
        }
    };
}

W_REGISTER_ARGTYPE(QtTube::PluginException)
W_OBJECT_IMPL_INLINE(QtTube::PluginReply<T>, template<typename T>)
W_OBJECT_IMPL_INLINE(QtTube::PluginReply<void>)
