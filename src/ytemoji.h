#ifndef YTEMOJI_H
#define YTEMOJI_H
#include <mutex>
#include <QJsonValue>

class QJsonArray;

class ytemoji
{
public:
    struct UnicodeEmoji
    {
        QString emojiId;
        QString image;
        QStringList searchTerms;
        QStringList shortcuts;
        bool supportsSkinTone;

        explicit UnicodeEmoji(const QJsonValue& emojiJson);
    };

    struct YouTubeEmoji
    {
        QString emojiId;
        bool hidden;
        QString image;
        QString shortcut;

        YouTubeEmoji(const QString& shortcut, const QString& emojiId, const QString& image, bool hidden = false)
            : shortcut(shortcut), emojiId(emojiId), image(image), hidden(hidden) {}
    };

    static ytemoji* instance();
    ytemoji();

    QString emojize(QString s, bool escape = true);
    QJsonArray produceRichText(QString s);

    QList<UnicodeEmoji> unicodeEmojis() const { return m_unicodeEmojis; }
    QList<YouTubeEmoji> youtubeEmojis() const { return m_youtubeEmojis; }
private:
    static inline ytemoji* m_instance;
    static inline std::once_flag m_onceFlag;
    const QList<YouTubeEmoji> m_youtubeEmojis = {
        YouTubeEmoji(":hand-pink-waving:", "UCkszU2WH9gy1mb0dV-11UJg/G8AfY6yWGuKuhL0PlbiA2AE", "https://yt3.ggpht.com/KOxdr_z3A5h1Gb7kqnxqOCnbZrBmxI2B_tRQ453BhTWUhYAlpg5ZP8IKEBkcvRoY8grY91Q=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-smiling:", "UCkszU2WH9gy1mb0dV-11UJg/KsIfY6LzFoLM6AKanYDQAg", "https://yt3.ggpht.com/cktIaPxFwnrPwn-alHvnvedHLUJwbHi8HCK3AgbHpphrMAW99qw0bDfxuZagSY5ieE9BBrA=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-red-droopy-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/W8IfY_bwAfiPq7IPvNCA2AU", "https://yt3.ggpht.com/oih9s26MOYPWC_uL6tgaeOlXSGBv8MMoDrWzBt-80nEiVSL9nClgnuzUAKqkU9_TWygF6CI=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-purple-crying:", "UCkszU2WH9gy1mb0dV-11UJg/b8IfY7zOK9iVkNAP_I2A-AY", "https://yt3.ggpht.com/g6_km98AfdHbN43gvEuNdZ2I07MmzVpArLwEvNBwwPqpZYzszqhRzU_DXALl11TchX5_xFE=w48-h48-c-k-nd"),
        YouTubeEmoji(":text-green-game-over:", "UCkszU2WH9gy1mb0dV-11UJg/hcIfY57lBJXp6AKBx4CoCA", "https://yt3.ggpht.com/cr36FHhSiMAJUSpO9XzjbOgxhtrdJNTVJUlMJeOOfLOFzKleAKT2SEkZwbqihBqfTXYCIg=w48-h48-c-k-nd"),
        YouTubeEmoji(":person-turquoise-waving:", "UCkszU2WH9gy1mb0dV-11UJg/ssIfY7OFG5OykQOpn4CQCw", "https://yt3.ggpht.com/uNSzQ2M106OC1L3VGzrOsGNjopboOv-m1bnZKFGuh0DxcceSpYHhYbuyggcgnYyaF3o-AQ=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-green-smiling:", "UCkszU2WH9gy1mb0dV-11UJg/xsIfY4OqCd2T29sP54iAsAw", "https://yt3.ggpht.com/G061SAfXg2bmG1ZXbJsJzQJpN8qEf_W3f5cb5nwzBYIV58IpPf6H90lElDl85iti3HgoL3o=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-orange-frowning:", "UCkszU2WH9gy1mb0dV-11UJg/2sIfY8vIG8z96ALulYDQDQ", "https://yt3.ggpht.com/Ar8jaEIxzfiyYmB7ejDOHba2kUMdR37MHn_R39mtxqO5CD4aYGvjDFL22DW_Cka6LKzhGDk=w48-h48-c-k-nd"),
        YouTubeEmoji(":eyes-purple-crying:", "UCkszU2WH9gy1mb0dV-11UJg/7cIfY5niDOmSkNAP08CA6A4", "https://yt3.ggpht.com/FrYgdeZPpvXs-6Mp305ZiimWJ0wV5bcVZctaUy80mnIdwe-P8HRGYAm0OyBtVx8EB9_Dxkc=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-fuchsia-wide-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/A8MfY-_pEIKNr8oP78-AGA", "https://yt3.ggpht.com/zdcOC1SMmyXJOAddl9DYeEFN9YYcn5mHemJCdRFQMtDuS0V-IyE-5YjNUL1tduX1zs17tQ=w48-h48-c-k-nd"),
        YouTubeEmoji(":cat-orange-whistling:", "UCkszU2WH9gy1mb0dV-11UJg/E8MfY5u7JPSXkNAP95GAmAE", "https://yt3.ggpht.com/0ocqEmuhrKCK87_J21lBkvjW70wRGC32-Buwk6TP4352CgcNjL6ug8zcsel6JiPbE58xhq5g=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-wide-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/LsMfY8P6G-yckNAPjoWA8AI", "https://yt3.ggpht.com/2Ht4KImoWDlCddiDQVuzSJwpEb59nZJ576ckfaMh57oqz2pUkkgVTXV8osqUOgFHZdUISJM=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-orange-raised-eyebrow:", "UCkszU2WH9gy1mb0dV-11UJg/Z8MfY8mzLbnovwK5roC4Bg", "https://yt3.ggpht.com/JbCfmOgYI-mO17LPw8e_ycqbBGESL8AVP6i7ZsBOVLd3PEpgrfEuJ9rEGpP_unDcqgWSCg=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-fuchsia-tongue-out:", "UCkszU2WH9gy1mb0dV-11UJg/hcMfY5_zAbbxvwKLooCoCA", "https://yt3.ggpht.com/EURfJZi_heNulV3mfHzXBk8PIs9XmZ9lOOYi5za6wFMCGrps4i2BJX9j-H2gK6LIhW6h7sY=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-orange-biting-nails:", "UCkszU2WH9gy1mb0dV-11UJg/ygF1XpGUMMjk8gSDrI2wCx", "https://yt3.ggpht.com/HmsXEgqUogkQOnL5LP_FdPit9Z909RJxby-uYcPxBLNhaPyqPTcGwvGaGPk2hzB_cC0hs_pV=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-red-heart-shape:", "UCkszU2WH9gy1mb0dV-11UJg/m8MfY4jbFsWJhL0PyouA2Ak", "https://yt3.ggpht.com/I0Mem9dU_IZ4a9cQPzR0pUJ8bH-882Eg0sDQjBmPcHA6Oq0uXOZcsjPvPbtormx91Ha2eRA=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-fuchsia-poop-shape:", "UCkszU2WH9gy1mb0dV-11UJg/6_cfY8HJH8bV5QS5yYDYDg", "https://yt3.ggpht.com/_xlyzvSimqMzhdhODyqUBLXIGA6F_d5en2bq-AIfc6fc3M7tw2jucuXRIo5igcW3g9VVe3A=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-purple-wide-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/DfgfY9LaNdmMq7IPuI2AaA", "https://yt3.ggpht.com/5RDrtjmzRQKuVYE_FKPUHiGh7TNtX5eSNe6XzcSytMsHirXYKunxpyAsVacTFMg0jmUGhQ=w48-h48-c-k-nd"),
        YouTubeEmoji(":glasses-purple-yellow-diamond:", "UCkszU2WH9gy1mb0dV-11UJg/HvgfY93GEYmqvwLUuYDwAQ", "https://yt3.ggpht.com/EnDBiuksboKsLkxp_CqMWlTcZtlL77QBkbjz_rLedMSDzrHmy_6k44YWFy2rk4I0LG6K2KI=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-pink-tears:", "UCkszU2WH9gy1mb0dV-11UJg/NvgfY9aeC_OFvOMPkrOAsAM", "https://yt3.ggpht.com/RL5QHCNcO_Mc98SxFEblXZt9FNoh3bIgsjm0Kj8kmeQJWMeTu7JX_NpICJ6KKwKT0oVHhAA=w48-h48-c-k-nd"),
        YouTubeEmoji(":body-blue-raised-arms:", "UCkszU2WH9gy1mb0dV-11UJg/UvgfY_vqE92T29sPvqiAkAU", "https://yt3.ggpht.com/2Jds3I9UKOfgjid97b_nlDU4X2t5MgjTof8yseCp7M-6ZhOhRkPGSPfYwmE9HjCibsfA1Uzo=w48-h48-c-k-nd"),
        YouTubeEmoji(":hand-orange-covering-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/YvgfY-LIBpjChgHKyYCQBg", "https://yt3.ggpht.com/y8ppa6GcJoRUdw7GwmjDmTAnSkeIkUptZMVQuFmFaTlF_CVIL7YP7hH7hd0TJbd8p9w67IM=w48-h48-c-k-nd"),
        YouTubeEmoji(":trophy-yellow-smiling:", "UCkszU2WH9gy1mb0dV-11UJg/ePgfY-K2Kp6Mr8oP1oqAwAc", "https://yt3.ggpht.com/7tf3A_D48gBg9g2N0Rm6HWs2aqzshHU4CuVubTXVxh1BP7YDBRC6pLBoC-ibvr-zCl_Lgg=w48-h48-c-k-nd"),
        YouTubeEmoji(":eyes-pink-heart-shape:", "UCkszU2WH9gy1mb0dV-11UJg/jPgfY5j2IIud29sP3ZeA4Ag", "https://yt3.ggpht.com/5vzlCQfQQdzsG7nlQzD8eNjtyLlnATwFwGvrMpC8dgLcosNhWLXu8NN9qIS3HZjJYd872dM=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-turquoise-covering-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/oPgfY_DoKfSXkNAPq8-AgAo", "https://yt3.ggpht.com/H2HNPRO8f4SjMmPNh5fl10okSETW7dLTZtuE4jh9D6pSmaUiLfoZJ2oiY-qWU3Owfm1IsXg=w48-h48-c-k-nd"),
        YouTubeEmoji(":hand-green-crystal-ball:", "UCkszU2WH9gy1mb0dV-11UJg/tPgfY7mSO4XovQKzmYCgCw", "https://yt3.ggpht.com/qZfJrWDEmR03FIak7PMNRNpMjNsCnOzD9PqK8mOpAp4Kacn_uXRNJNb99tE_1uyEbvgJReF2=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-turquoise-drinking-coffee:", "UCkszU2WH9gy1mb0dV-11UJg/zPgfY66lCJGRhL0Pz6iA4Aw", "https://yt3.ggpht.com/myqoI1MgFUXQr5fuWTC9mz0BCfgf3F8GSDp06o1G7w6pTz48lwARjdG8vj0vMxADvbwA1dA=w48-h48-c-k-nd"),
        YouTubeEmoji(":body-green-covering-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/4PgfY73cJprKCq-_gIAO", "https://yt3.ggpht.com/UR8ydcU3gz360bzDsprB6d1klFSQyVzgn-Fkgu13dIKPj3iS8OtG1bhBUXPdj9pMwtM00ro=w48-h48-c-k-nd"),
        YouTubeEmoji(":goat-turquoise-white-horns:", "UCkszU2WH9gy1mb0dV-11UJg/-fgfY9DIGYjbhgHLzoDIDw", "https://yt3.ggpht.com/jMnX4lu5GnjBRgiPtX5FwFmEyKTlWFrr5voz-Auko35oP0t3-zhPxR3PQMYa-7KhDeDtrv4=w48-h48-c-k-nd"),
        YouTubeEmoji(":hand-purple-blue-peace:", "UCkszU2WH9gy1mb0dV-11UJg/EvkfY6uNC5OykQOewoCQAQ", "https://yt3.ggpht.com/-sC8wj6pThd7FNdslEoJlG4nB9SIbrJG3CRGh7-bNV0RVfcrJuwiWHoUZ6UmcVs7sQjxTg4=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-question-mark:", "UCkszU2WH9gy1mb0dV-11UJg/LfkfY_zhH4GFr8oP4aKA6AI", "https://yt3.ggpht.com/Wx4PMqTwG3f4gtR7J9Go1s8uozzByGWLSXHzrh3166ixaYRinkH_F05lslfsRUsKRvHXrDk=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-covering-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/RPkfY8TPGsCakNAP-JWAoAQ", "https://yt3.ggpht.com/kj3IgbbR6u-mifDkBNWVcdOXC-ut-tiFbDpBMGVeW79c2c54n5vI-HNYCOC6XZ9Bzgupc10=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-purple-smiling-fangs:", "UCkszU2WH9gy1mb0dV-11UJg/Mm5IY53bH7SEq7IP-MWAkAM", "https://yt3.ggpht.com/k1vqi6xoHakGUfa0XuZYWHOv035807ARP-ZLwFmA-_NxENJMxsisb-kUgkSr96fj5baBOZE=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-purple-sweating:", "UCkszU2WH9gy1mb0dV-11UJg/UW5IY-ibBqa8jgTymoCIBQ", "https://yt3.ggpht.com/tRnrCQtEKlTM9YLPo0vaxq9mDvlT0mhDld2KI7e_nDRbhta3ULKSoPVHZ1-bNlzQRANmH90=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-purple-smiling-tears:", "UCkszU2WH9gy1mb0dV-11UJg/Ym5IY7-0LoqA29sPq9CAkAY", "https://yt3.ggpht.com/MJV1k3J5s0hcUfuo78Y6MKi-apDY5NVDjO9Q7hL8fU4i0cIBgU-cU4rq4sHessJuvuGpDOjJ=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-star-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/dG5IY-mhEof9jgSykoCgBw", "https://yt3.ggpht.com/m_ANavMhp6cQ1HzX0HCTgp_er_yO2UA28JPbi-0HElQgnQ4_q5RUhgwueTpH-st8L3MyTA=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-heart-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/hm5IY4W-H9SO5QS6n4CwCA", "https://yt3.ggpht.com/M9tzKd64_r3hvgpTSgca7K3eBlGuyiqdzzhYPp7ullFAHMgeFoNLA0uQ1dGxj3fXgfcHW4w=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-three-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/mW5IY47PMcSnkMkPo6OAyAk", "https://yt3.ggpht.com/nSQHitVplLe5uZC404dyAwv1f58S3PN-U_799fvFzq-6b3bv-MwENO-Zs1qQI4oEXCbOJg=w48-h48-c-k-nd"),
        YouTubeEmoji(":face-blue-droopy-eyes:", "UCkszU2WH9gy1mb0dV-11UJg/rW5IY_26FryOq7IPlL2A6Ao", "https://yt3.ggpht.com/hGPqMUCiXGt6zuX4dHy0HRZtQ-vZmOY8FM7NOHrJTta3UEJksBKjOcoE6ZUAW9sz7gIF_nk=w48-h48-c-k-nd"),
        YouTubeEmoji(":planet-orange-purple-ring:", "UCkszU2WH9gy1mb0dV-11UJg/v25IY7KcJIGOr8oPz4OA-As", "https://yt3.ggpht.com/xkaLigm3P4_1g4X1JOtkymcC7snuJu_C5YwIFAyQlAXK093X0IUjaSTinMTLKeRZ6280jXg=w48-h48-c-k-nd"),
        YouTubeEmoji(":yt:", "UCkszU2WH9gy1mb0dV-11UJg/CIW60IPp_dYCFcuqTgodEu4IlQ", "https://yt3.ggpht.com/IkpeJf1g9Lq0WNjvSa4XFq4LVNZ9IP5FKW8yywXb12djo1OGdJtziejNASITyq4L0itkMNw=w48-h48-c-k-nd"),
        YouTubeEmoji(":oops:", "UCkszU2WH9gy1mb0dV-11UJg/CN2m5cKr49sCFYbFggodDFEKrg", "https://yt3.ggpht.com/PFoVIqIiFRS3aFf5-bt_tTC0WrDm_ylhF4BKKwgqAASNb7hVgx_adFP-XVhFiJLXdRK0EQ=w48-h48-c-k-nd"),
        YouTubeEmoji(":buffering:", "UCkszU2WH9gy1mb0dV-11UJg/X_zdXMHgJaPa8gTGt4f4Ag", "https://yt3.ggpht.com/5gfMEfdqO9CiLwhN9Mq7VI6--T2QFp8AXNNy5Fo7btfY6fRKkThWq35SCZ6SPMVCjg-sUA=w48-h48-c-k-nd"),
        YouTubeEmoji(":stayhome:", "UCkszU2WH9gy1mb0dV-11UJg/1v50XorRJ8GQ8gTz_prwAg", "https://yt3.ggpht.com/_1FGHypiub51kuTiNBX1a0H3NyFih3TnHX7bHU06j_ajTzT0OQfMLl9RI1SiQoxtgA2Grg=w48-h48-c-k-nd"),
        YouTubeEmoji(":dothefive:", "UCkszU2WH9gy1mb0dV-11UJg/8P50XuS9Oo7h8wSqtIagBA", "https://yt3.ggpht.com/-nM0DOd49969h3GNcl705Ti1fIf1ZG_E3JxcOUVV-qPfCW6jY8xZ98caNLHkVSGRTSEb7Y9y=w48-h48-c-k-nd"),
        YouTubeEmoji(":elbowbump:", "UCkszU2WH9gy1mb0dV-11UJg/Fv90Xq-vJcPq8gTqzreQAQ", "https://yt3.ggpht.com/2ou58X5XuhTrxjtIM2wew1f-HKRhN_T5SILQgHE-WD9dySzzJdGwL4R1gpKiJXcbtq6sjQ=w48-h48-c-k-nd"),
        YouTubeEmoji(":goodvibes:", "UCkszU2WH9gy1mb0dV-11UJg/Iv90XouTLuOR8gSxxrToBA", "https://yt3.ggpht.com/2CvFOwgKpL29mW_C51XvaWa7Eixtv-3tD1XvZa1_WemaDDL2AqevKbTZ1rdV0OWcnOZRag=w48-h48-c-k-nd"),
        YouTubeEmoji(":thanksdoc:", "UCkszU2WH9gy1mb0dV-11UJg/Rf90XtDbG8GQ8gTz_prwAg", "https://yt3.ggpht.com/bUnO_VwXW2hDf-Da8D64KKv6nBJDYUBuo13RrOg141g2da8pi9-KClJYlUDuqIwyPBfvOO8=w48-h48-c-k-nd"),
        YouTubeEmoji(":videocall:", "UCkszU2WH9gy1mb0dV-11UJg/VP90Xv_wG82o8wTCi7CQAw", "https://yt3.ggpht.com/k5v_oxUzRWmTOXP0V6WJver6xdS1lyHMPcMTfxn23Md6rmixoR5RZUusFbZi1uZwjF__pv4=w48-h48-c-k-nd"),
        YouTubeEmoji(":virtualhug:", "UCkszU2WH9gy1mb0dV-11UJg/dv90XtfhAurw8gTgzar4DA", "https://yt3.ggpht.com/U1TjOZlqtS58NGqQhE8VWDptPSrmJNkrbVRp_8jI4f84QqIGflq2Ibu7YmuOg5MmVYnpevc=w48-h48-c-k-nd"),
        YouTubeEmoji(":yougotthis:", "UCkszU2WH9gy1mb0dV-11UJg/hf90Xv-jHeOR8gSxxrToBA", "https://yt3.ggpht.com/s3uOe4lUx3iPIt1h901SlMp_sKCTp3oOVj1JV8izBw_vDVLxFqk5dq-3NX-nK_gnUwVEXld3=w48-h48-c-k-nd"),
        YouTubeEmoji(":sanitizer:", "UCkszU2WH9gy1mb0dV-11UJg/lP90XvOhCZGl8wSO1JmgAw", "https://yt3.ggpht.com/EJ_8vc4Gl-WxCWBurHwwWROAHrPzxgePodoNfkRY1U_I8L1O2zlqf7-wfUtTeyzq2qHNnocZ=w48-h48-c-k-nd"),
        YouTubeEmoji(":takeout:", "UCkszU2WH9gy1mb0dV-11UJg/uP90Xq6wNYrK8gTUoo3wAg", "https://yt3.ggpht.com/FizHI5IYMoNql9XeP7TV3E0ffOaNKTUSXbjtJe90e1OUODJfZbWU37VqBbTh-vpyFHlFIS0=w48-h48-c-k-nd"),
        YouTubeEmoji(":hydrate:", "UCkszU2WH9gy1mb0dV-11UJg/fAF1XtDQMIrK8gTUoo3wAg", "https://yt3.ggpht.com/tpgZgmhX8snKniye36mnrDVfTnlc44EK92EPeZ0m9M2EPizn1vKEGJzNYdp7KQy6iNZlYDc1=w48-h48-c-k-nd"),
        YouTubeEmoji(":chillwcat:", "UCkszU2WH9gy1mb0dV-11UJg/vQF1XpyaG_XG8gTs77bACQ", "https://yt3.ggpht.com/y03dFcPc1B7CO20zgQYzhcRPka5Bhs6iSg57MaxJdhaLidFvvXBLf_i4_SHG7zJ_2VpBMNs=w48-h48-c-k-nd"),
        YouTubeEmoji(":chillwdog", "UCkszU2WH9gy1mb0dV-11UJg/ygF1XpGUMMjk8gSDrI2wCw", "https://yt3.ggpht.com/Ir9mDxzUi0mbqyYdJ3N9Lq7bN5Xdt0Q7fEYFngN3GYAcJT_tccH1as1PKmInnpt2cbWOam4=w48-h48-c-k-nd"),
        YouTubeEmoji(":elbowcough:", "UCkszU2WH9gy1mb0dV-11UJg/8gF1Xp_zK8jk8gSDrI2wCw", "https://yt3.ggpht.com/DTR9bZd1HOqpRJyz9TKiLb0cqe5Hb84Yi_79A6LWlN1tY-5kXqLDXRmtYVKE9rcqzEghmw=w24-h24-c-k-nd"),
        YouTubeEmoji(":learning:", "UCkszU2WH9gy1mb0dV-11UJg/EAJ1XrS7PMGQ8gTz_prwAg", "https://yt3.ggpht.com/ZuBuz8GAQ6IEcQc7CoJL8IEBTYbXEvzhBeqy1AiytmhuAT0VHjpXEjd-A5GfR4zDin1L53Q=w48-h48-c-k-nd"),
        YouTubeEmoji(":washhands:", "UCkszU2WH9gy1mb0dV-11UJg/JAJ1XpGpJYnW8wTupZu4Cw", "https://yt3.ggpht.com/qXUeUW0KpKBc9Z3AqUqr_0B7HbW1unAv4qmt7-LJGUK_gsFBIaHISWJNt4n3yvmAnQNZHE-u=w48-h48-c-k-nd"),
        YouTubeEmoji(":socialdist:", "UCkszU2WH9gy1mb0dV-11UJg/PAJ1XsOOI4fegwOo57ewAg", "https://yt3.ggpht.com/igBNi55-TACUi1xQkqMAor-IEXmt8He56K7pDTG5XoTsbM-rVswNzUfC5iwnfrpunWihrg=w48-h48-c-k-nd"),
        YouTubeEmoji(":shelterin:", "UCkszU2WH9gy1mb0dV-11UJg/egJ1XufTKYfegwOo57ewAg", "https://yt3.ggpht.com/gjC5x98J4BoVSEPfFJaoLtc4tSBGSEdIlfL2FV4iJG9uGNykDP9oJC_QxAuBTJy6dakPxVeC=w48-h48-c-k-nd")
    };

    QList<UnicodeEmoji> m_unicodeEmojis;
};

#endif // YTEMOJI_H
