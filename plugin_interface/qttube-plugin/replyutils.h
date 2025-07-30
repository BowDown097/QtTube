#pragma once
#include "qttube-plugin/components/reply.h"
#include <bitset>
#include <QMutexLocker>

namespace QtTubePlugin
{
    template<typename T>
    inline constexpr bool is_reply_v = false;

    template<typename T>
    inline constexpr bool is_reply_v<Reply<T>> = true;

    // this should be used when returning results immediately in a reply.
    // this is quite spaghetti tho, i don't like it :(
    template<typename ReplyType> requires is_reply_v<ReplyType>
    void delayedEmit(ReplyType* inst, auto&&... args)
    {
        using FirstArg = std::remove_cvref_t<std::tuple_element_t<0, std::tuple<decltype(args)...>>>;
        QMetaObject::invokeMethod(inst, [inst, ...args = std::forward<decltype(args)>(args)]() mutable {
            if constexpr (std::same_as<FirstArg, Exception>)
                emit inst->exception(std::forward<decltype(args)>(args)...);
            else
                emit inst->finished(std::forward<decltype(args)>(args)...);
        }, Qt::QueuedConnection);
    }

    // small utility class to handle completion tracking of multiple concurrent tasks
    template<size_t N>
    class MultiCompletionState
    {
    public:
        bool hit()
        {
            QMutexLocker locker(&mutex);
            fbits.set(position++);
            return fbits.all();
        }
    private:
        std::bitset<N> fbits;
        QMutex mutex;
        size_t position{};
    };
}
