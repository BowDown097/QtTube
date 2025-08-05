#pragma once
#include <any>
#include <QException>
#include <wobjectimpl.h>

namespace QtTubePlugin
{
    class Exception : public QException
    {
    public:
        enum class Severity { Normal, Minor };
        explicit Exception(const QString& message, Severity severity = Severity::Normal)
            : m_message(message), m_severity(severity) {}
        explicit Exception(QString&& message, Severity severity = Severity::Normal)
            : m_message(std::move(message)), m_severity(severity) {}
        const QString& message() const { return m_message; }
        Severity severity() const { return m_severity; }
    private:
        QString m_message;
        Severity m_severity;
    };

    template<typename T>
    class Reply : public QObject
    {
        W_OBJECT(Reply)
    public:
        using data_type = T;

        std::any continuationData;

        void exception(const QtTubePlugin::Exception& ex) W_SIGNAL(exception, ex)
        void finished(const T& object) W_SIGNAL(finished, object)

        // creates a reply that is automatically deleted when finished. you'll want to use this with multithreaded code.
        static Reply<T>* create()
        {
            Reply<T>* reply = new Reply<T>;
            connect(reply, &Reply<T>::exception, reply, &QObject::deleteLater);
            connect(reply, &Reply<T>::finished, reply, &QObject::deleteLater);
            return reply;
        }
    private:
        Reply() = default;
    };

    template<>
    class Reply<void> : public QObject
    {
        W_OBJECT(Reply<void>)
    public:
        using data_type = void;

        std::any continuationData;

        void exception(const QtTubePlugin::Exception& ex) W_SIGNAL(exception, ex)
        void finished() W_SIGNAL(finished)

        // creates a reply that is automatically deleted when finished. you'll want to use this with multithreaded code.
        static Reply<void>* create()
        {
            Reply<void>* reply = new Reply<void>;
            connect(reply, &Reply<void>::exception, reply, &QObject::deleteLater);
            connect(reply, &Reply<void>::finished, reply, &QObject::deleteLater);
            return reply;
        }
    private:
        Reply() = default;
    };
}

W_REGISTER_ARGTYPE(QtTubePlugin::Exception)
W_OBJECT_IMPL_INLINE(QtTubePlugin::Reply<T>, template<typename T>)
W_OBJECT_IMPL_INLINE(QtTubePlugin::Reply<void>)
