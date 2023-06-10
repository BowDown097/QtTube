#include "emoji.h"

namespace emojicpp
{
    QString emojize(QString s, bool escape) {
        int index = -1;
        int sLen = s.size();
        for (int i = 0; i < sLen; i++) {
            if (s[i] == ':') {
                // check if colon is escaped
                if(escape && i!=0 && s[i-1]=='\\')
                    continue;
                if (index == -1) {
                    index = i;
                }
                else {
                    if (i - index==1) {
                        index = i;
                        continue;
                    }
                    QMap<QString, QString>::const_iterator it = EMOJIS.constFind(s.mid(index, i - index + 1));
                    if (it == EMOJIS.constEnd()) {
                        index = i;
                        continue;
                    }
                    const QString& emo = it.value();
                    // replace from index to i
                    //std::cout << s.substr(index, i - index + 1) << std::endl; // <---- uncomment to see what text is replaced, might be good for debugging
                    s.replace(index, i - index + 1, emo);
                    int goBack = i - index + 1 - emo.size();
                    sLen -= goBack;
                    i -= goBack;
                    index = -1;
                }
            }
        }
        return s;
    }
}
