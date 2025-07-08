#include "replydata.h"
#include "conversion.h"
#include "httprequest.h"
#include "innertube.h"
#include "utils/stringutils.h"
#include "utils/tubeutils.h"
#include <QJsonDocument>

const QList<QtTubePlugin::Emoji> g_platformEmojis = {
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/G8AfY6yWGuKuhL0PlbiA2AE", .shortcodes = {":hand-pink-waving:"}, .url = "https://yt3.ggpht.com/KOxdr_z3A5h1Gb7kqnxqOCnbZrBmxI2B_tRQ453BhTWUhYAlpg5ZP8IKEBkcvRoY8grY91Q=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/KsIfY6LzFoLM6AKanYDQAg", .shortcodes = {":face-blue-smiling:"}, .url = "https://yt3.ggpht.com/cktIaPxFwnrPwn-alHvnvedHLUJwbHi8HCK3AgbHpphrMAW99qw0bDfxuZagSY5ieE9BBrA=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/W8IfY_bwAfiPq7IPvNCA2AU", .shortcodes = {":face-red-droopy-eyes:"}, .url = "https://yt3.ggpht.com/oih9s26MOYPWC_uL6tgaeOlXSGBv8MMoDrWzBt-80nEiVSL9nClgnuzUAKqkU9_TWygF6CI=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/b8IfY7zOK9iVkNAP_I2A-AY", .shortcodes = {":face-purple-crying:"}, .url = "https://yt3.ggpht.com/g6_km98AfdHbN43gvEuNdZ2I07MmzVpArLwEvNBwwPqpZYzszqhRzU_DXALl11TchX5_xFE=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/hcIfY57lBJXp6AKBx4CoCA", .shortcodes = {":text-green-game-over:"}, .url = "https://yt3.ggpht.com/cr36FHhSiMAJUSpO9XzjbOgxhtrdJNTVJUlMJeOOfLOFzKleAKT2SEkZwbqihBqfTXYCIg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/ssIfY7OFG5OykQOpn4CQCw", .shortcodes = {":person-turquoise-waving:"}, .url = "https://yt3.ggpht.com/uNSzQ2M106OC1L3VGzrOsGNjopboOv-m1bnZKFGuh0DxcceSpYHhYbuyggcgnYyaF3o-AQ=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/xsIfY4OqCd2T29sP54iAsAw", .shortcodes = {":face-green-smiling:"}, .url = "https://yt3.ggpht.com/G061SAfXg2bmG1ZXbJsJzQJpN8qEf_W3f5cb5nwzBYIV58IpPf6H90lElDl85iti3HgoL3o=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/2sIfY8vIG8z96ALulYDQDQ", .shortcodes = {":face-orange-frowning:"}, .url = "https://yt3.ggpht.com/Ar8jaEIxzfiyYmB7ejDOHba2kUMdR37MHn_R39mtxqO5CD4aYGvjDFL22DW_Cka6LKzhGDk=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/7cIfY5niDOmSkNAP08CA6A4", .shortcodes = {":eyes-purple-crying:"}, .url = "https://yt3.ggpht.com/FrYgdeZPpvXs-6Mp305ZiimWJ0wV5bcVZctaUy80mnIdwe-P8HRGYAm0OyBtVx8EB9_Dxkc=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/A8MfY-_pEIKNr8oP78-AGA", .shortcodes = {":face-fuchsia-wide-eyes:"}, .url = "https://yt3.ggpht.com/zdcOC1SMmyXJOAddl9DYeEFN9YYcn5mHemJCdRFQMtDuS0V-IyE-5YjNUL1tduX1zs17tQ=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/E8MfY5u7JPSXkNAP95GAmAE", .shortcodes = {":cat-orange-whistling:"}, .url = "https://yt3.ggpht.com/0ocqEmuhrKCK87_J21lBkvjW70wRGC32-Buwk6TP4352CgcNjL6ug8zcsel6JiPbE58xhq5g=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/LsMfY8P6G-yckNAPjoWA8AI", .shortcodes = {":face-blue-wide-eyes:"}, .url = "https://yt3.ggpht.com/2Ht4KImoWDlCddiDQVuzSJwpEb59nZJ576ckfaMh57oqz2pUkkgVTXV8osqUOgFHZdUISJM=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/Z8MfY8mzLbnovwK5roC4Bg", .shortcodes = {":face-orange-raised-eyebrow:"}, .url = "https://yt3.ggpht.com/JbCfmOgYI-mO17LPw8e_ycqbBGESL8AVP6i7ZsBOVLd3PEpgrfEuJ9rEGpP_unDcqgWSCg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/hcMfY5_zAbbxvwKLooCoCA", .shortcodes = {":face-fuchsia-tongue-out:"}, .url = "https://yt3.ggpht.com/EURfJZi_heNulV3mfHzXBk8PIs9XmZ9lOOYi5za6wFMCGrps4i2BJX9j-H2gK6LIhW6h7sY=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/ygF1XpGUMMjk8gSDrI2wCx", .shortcodes = {":face-orange-biting-nails:"}, .url = "https://yt3.ggpht.com/HmsXEgqUogkQOnL5LP_FdPit9Z909RJxby-uYcPxBLNhaPyqPTcGwvGaGPk2hzB_cC0hs_pV=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/m8MfY4jbFsWJhL0PyouA2Ak", .shortcodes = {":face-red-heart-shape:"}, .url = "https://yt3.ggpht.com/I0Mem9dU_IZ4a9cQPzR0pUJ8bH-882Eg0sDQjBmPcHA6Oq0uXOZcsjPvPbtormx91Ha2eRA=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/6_cfY8HJH8bV5QS5yYDYDg", .shortcodes = {":face-fuchsia-poop-shape:"}, .url = "https://yt3.ggpht.com/_xlyzvSimqMzhdhODyqUBLXIGA6F_d5en2bq-AIfc6fc3M7tw2jucuXRIo5igcW3g9VVe3A=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/DfgfY9LaNdmMq7IPuI2AaA", .shortcodes = {":face-purple-wide-eyes:"}, .url = "https://yt3.ggpht.com/5RDrtjmzRQKuVYE_FKPUHiGh7TNtX5eSNe6XzcSytMsHirXYKunxpyAsVacTFMg0jmUGhQ=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/HvgfY93GEYmqvwLUuYDwAQ", .shortcodes = {":glasses-purple-yellow-diamond:"}, .url = "https://yt3.ggpht.com/EnDBiuksboKsLkxp_CqMWlTcZtlL77QBkbjz_rLedMSDzrHmy_6k44YWFy2rk4I0LG6K2KI=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/NvgfY9aeC_OFvOMPkrOAsAM", .shortcodes = {":face-pink-tears:"}, .url = "https://yt3.ggpht.com/RL5QHCNcO_Mc98SxFEblXZt9FNoh3bIgsjm0Kj8kmeQJWMeTu7JX_NpICJ6KKwKT0oVHhAA=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/UvgfY_vqE92T29sPvqiAkAU", .shortcodes = {":body-blue-raised-arms:"}, .url = "https://yt3.ggpht.com/2Jds3I9UKOfgjid97b_nlDU4X2t5MgjTof8yseCp7M-6ZhOhRkPGSPfYwmE9HjCibsfA1Uzo=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/YvgfY-LIBpjChgHKyYCQBg", .shortcodes = {":hand-orange-covering-eyes:"}, .url = "https://yt3.ggpht.com/y8ppa6GcJoRUdw7GwmjDmTAnSkeIkUptZMVQuFmFaTlF_CVIL7YP7hH7hd0TJbd8p9w67IM=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/ePgfY-K2Kp6Mr8oP1oqAwAc", .shortcodes = {":trophy-yellow-smiling:"}, .url = "https://yt3.ggpht.com/7tf3A_D48gBg9g2N0Rm6HWs2aqzshHU4CuVubTXVxh1BP7YDBRC6pLBoC-ibvr-zCl_Lgg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/jPgfY5j2IIud29sP3ZeA4Ag", .shortcodes = {":eyes-pink-heart-shape:"}, .url = "https://yt3.ggpht.com/5vzlCQfQQdzsG7nlQzD8eNjtyLlnATwFwGvrMpC8dgLcosNhWLXu8NN9qIS3HZjJYd872dM=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/oPgfY_DoKfSXkNAPq8-AgAo", .shortcodes = {":face-turquoise-covering-eyes:"}, .url = "https://yt3.ggpht.com/H2HNPRO8f4SjMmPNh5fl10okSETW7dLTZtuE4jh9D6pSmaUiLfoZJ2oiY-qWU3Owfm1IsXg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/tPgfY7mSO4XovQKzmYCgCw", .shortcodes = {":hand-green-crystal-ball:"}, .url = "https://yt3.ggpht.com/qZfJrWDEmR03FIak7PMNRNpMjNsCnOzD9PqK8mOpAp4Kacn_uXRNJNb99tE_1uyEbvgJReF2=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/zPgfY66lCJGRhL0Pz6iA4Aw", .shortcodes = {":face-turquoise-drinking-coffee:"}, .url = "https://yt3.ggpht.com/myqoI1MgFUXQr5fuWTC9mz0BCfgf3F8GSDp06o1G7w6pTz48lwARjdG8vj0vMxADvbwA1dA=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/4PgfY73cJprKCq-_gIAO", .shortcodes = {":body-green-covering-eyes:"}, .url = "https://yt3.ggpht.com/UR8ydcU3gz360bzDsprB6d1klFSQyVzgn-Fkgu13dIKPj3iS8OtG1bhBUXPdj9pMwtM00ro=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/-fgfY9DIGYjbhgHLzoDIDw", .shortcodes = {":goat-turquoise-white-horns:"}, .url = "https://yt3.ggpht.com/jMnX4lu5GnjBRgiPtX5FwFmEyKTlWFrr5voz-Auko35oP0t3-zhPxR3PQMYa-7KhDeDtrv4=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/EvkfY6uNC5OykQOewoCQAQ", .shortcodes = {":hand-purple-blue-peace:"}, .url = "https://yt3.ggpht.com/-sC8wj6pThd7FNdslEoJlG4nB9SIbrJG3CRGh7-bNV0RVfcrJuwiWHoUZ6UmcVs7sQjxTg4=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/LfkfY_zhH4GFr8oP4aKA6AI", .shortcodes = {":face-blue-question-mark:"}, .url = "https://yt3.ggpht.com/Wx4PMqTwG3f4gtR7J9Go1s8uozzByGWLSXHzrh3166ixaYRinkH_F05lslfsRUsKRvHXrDk=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/RPkfY8TPGsCakNAP-JWAoAQ", .shortcodes = {":face-blue-covering-eyes:"}, .url = "https://yt3.ggpht.com/kj3IgbbR6u-mifDkBNWVcdOXC-ut-tiFbDpBMGVeW79c2c54n5vI-HNYCOC6XZ9Bzgupc10=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/Mm5IY53bH7SEq7IP-MWAkAM", .shortcodes = {":face-purple-smiling-fangs:"}, .url = "https://yt3.ggpht.com/k1vqi6xoHakGUfa0XuZYWHOv035807ARP-ZLwFmA-_NxENJMxsisb-kUgkSr96fj5baBOZE=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/UW5IY-ibBqa8jgTymoCIBQ", .shortcodes = {":face-purple-sweating:"}, .url = "https://yt3.ggpht.com/tRnrCQtEKlTM9YLPo0vaxq9mDvlT0mhDld2KI7e_nDRbhta3ULKSoPVHZ1-bNlzQRANmH90=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/Ym5IY7-0LoqA29sPq9CAkAY", .shortcodes = {":face-purple-smiling-tears:"}, .url = "https://yt3.ggpht.com/MJV1k3J5s0hcUfuo78Y6MKi-apDY5NVDjO9Q7hL8fU4i0cIBgU-cU4rq4sHessJuvuGpDOjJ=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/dG5IY-mhEof9jgSykoCgBw", .shortcodes = {":face-blue-star-eyes:"}, .url = "https://yt3.ggpht.com/m_ANavMhp6cQ1HzX0HCTgp_er_yO2UA28JPbi-0HElQgnQ4_q5RUhgwueTpH-st8L3MyTA=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/hm5IY4W-H9SO5QS6n4CwCA", .shortcodes = {":face-blue-heart-eyes:"}, .url = "https://yt3.ggpht.com/M9tzKd64_r3hvgpTSgca7K3eBlGuyiqdzzhYPp7ullFAHMgeFoNLA0uQ1dGxj3fXgfcHW4w=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/mW5IY47PMcSnkMkPo6OAyAk", .shortcodes = {":face-blue-three-eyes:"}, .url = "https://yt3.ggpht.com/nSQHitVplLe5uZC404dyAwv1f58S3PN-U_799fvFzq-6b3bv-MwENO-Zs1qQI4oEXCbOJg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/rW5IY_26FryOq7IPlL2A6Ao", .shortcodes = {":face-blue-droopy-eyes:"}, .url = "https://yt3.ggpht.com/hGPqMUCiXGt6zuX4dHy0HRZtQ-vZmOY8FM7NOHrJTta3UEJksBKjOcoE6ZUAW9sz7gIF_nk=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/v25IY7KcJIGOr8oPz4OA-As", .shortcodes = {":planet-orange-purple-ring:"}, .url = "https://yt3.ggpht.com/xkaLigm3P4_1g4X1JOtkymcC7snuJu_C5YwIFAyQlAXK093X0IUjaSTinMTLKeRZ6280jXg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/CIW60IPp_dYCFcuqTgodEu4IlQ", .shortcodes = {":yt:"}, .url = "https://yt3.ggpht.com/IkpeJf1g9Lq0WNjvSa4XFq4LVNZ9IP5FKW8yywXb12djo1OGdJtziejNASITyq4L0itkMNw=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/CN2m5cKr49sCFYbFggodDFEKrg", .shortcodes = {":oops:"}, .url = "https://yt3.ggpht.com/PFoVIqIiFRS3aFf5-bt_tTC0WrDm_ylhF4BKKwgqAASNb7hVgx_adFP-XVhFiJLXdRK0EQ=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/X_zdXMHgJaPa8gTGt4f4Ag", .shortcodes = {":buffering:"}, .url = "https://yt3.ggpht.com/5gfMEfdqO9CiLwhN9Mq7VI6--T2QFp8AXNNy5Fo7btfY6fRKkThWq35SCZ6SPMVCjg-sUA=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/1v50XorRJ8GQ8gTz_prwAg", .shortcodes = {":stayhome:"}, .url = "https://yt3.ggpht.com/_1FGHypiub51kuTiNBX1a0H3NyFih3TnHX7bHU06j_ajTzT0OQfMLl9RI1SiQoxtgA2Grg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/8P50XuS9Oo7h8wSqtIagBA", .shortcodes = {":dothefive:"}, .url = "https://yt3.ggpht.com/-nM0DOd49969h3GNcl705Ti1fIf1ZG_E3JxcOUVV-qPfCW6jY8xZ98caNLHkVSGRTSEb7Y9y=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/Fv90Xq-vJcPq8gTqzreQAQ", .shortcodes = {":elbowbump:"}, .url = "https://yt3.ggpht.com/2ou58X5XuhTrxjtIM2wew1f-HKRhN_T5SILQgHE-WD9dySzzJdGwL4R1gpKiJXcbtq6sjQ=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/Iv90XouTLuOR8gSxxrToBA", .shortcodes = {":goodvibes:"}, .url = "https://yt3.ggpht.com/2CvFOwgKpL29mW_C51XvaWa7Eixtv-3tD1XvZa1_WemaDDL2AqevKbTZ1rdV0OWcnOZRag=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/Rf90XtDbG8GQ8gTz_prwAg", .shortcodes = {":thanksdoc:"}, .url = "https://yt3.ggpht.com/bUnO_VwXW2hDf-Da8D64KKv6nBJDYUBuo13RrOg141g2da8pi9-KClJYlUDuqIwyPBfvOO8=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/VP90Xv_wG82o8wTCi7CQAw", .shortcodes = {":videocall:"}, .url = "https://yt3.ggpht.com/k5v_oxUzRWmTOXP0V6WJver6xdS1lyHMPcMTfxn23Md6rmixoR5RZUusFbZi1uZwjF__pv4=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/dv90XtfhAurw8gTgzar4DA", .shortcodes = {":virtualhug:"}, .url = "https://yt3.ggpht.com/U1TjOZlqtS58NGqQhE8VWDptPSrmJNkrbVRp_8jI4f84QqIGflq2Ibu7YmuOg5MmVYnpevc=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/hf90Xv-jHeOR8gSxxrToBA", .shortcodes = {":yougotthis:"}, .url = "https://yt3.ggpht.com/s3uOe4lUx3iPIt1h901SlMp_sKCTp3oOVj1JV8izBw_vDVLxFqk5dq-3NX-nK_gnUwVEXld3=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/lP90XvOhCZGl8wSO1JmgAw", .shortcodes = {":sanitizer:"}, .url = "https://yt3.ggpht.com/EJ_8vc4Gl-WxCWBurHwwWROAHrPzxgePodoNfkRY1U_I8L1O2zlqf7-wfUtTeyzq2qHNnocZ=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/uP90Xq6wNYrK8gTUoo3wAg", .shortcodes = {":takeout:"}, .url = "https://yt3.ggpht.com/FizHI5IYMoNql9XeP7TV3E0ffOaNKTUSXbjtJe90e1OUODJfZbWU37VqBbTh-vpyFHlFIS0=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/fAF1XtDQMIrK8gTUoo3wAg", .shortcodes = {":hydrate:"}, .url = "https://yt3.ggpht.com/tpgZgmhX8snKniye36mnrDVfTnlc44EK92EPeZ0m9M2EPizn1vKEGJzNYdp7KQy6iNZlYDc1=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/vQF1XpyaG_XG8gTs77bACQ", .shortcodes = {":chillwcat:"}, .url = "https://yt3.ggpht.com/y03dFcPc1B7CO20zgQYzhcRPka5Bhs6iSg57MaxJdhaLidFvvXBLf_i4_SHG7zJ_2VpBMNs=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/ygF1XpGUMMjk8gSDrI2wCw", .shortcodes = {":chillwdog"}, .url = "https://yt3.ggpht.com/Ir9mDxzUi0mbqyYdJ3N9Lq7bN5Xdt0Q7fEYFngN3GYAcJT_tccH1as1PKmInnpt2cbWOam4=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/8gF1Xp_zK8jk8gSDrI2wCw", .shortcodes = {":elbowcough:"}, .url = "https://yt3.ggpht.com/DTR9bZd1HOqpRJyz9TKiLb0cqe5Hb84Yi_79A6LWlN1tY-5kXqLDXRmtYVKE9rcqzEghmw=w24-h24-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/EAJ1XrS7PMGQ8gTz_prwAg", .shortcodes = {":learning:"}, .url = "https://yt3.ggpht.com/ZuBuz8GAQ6IEcQc7CoJL8IEBTYbXEvzhBeqy1AiytmhuAT0VHjpXEjd-A5GfR4zDin1L53Q=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/JAJ1XpGpJYnW8wTupZu4Cw", .shortcodes = {":washhands:"}, .url = "https://yt3.ggpht.com/qXUeUW0KpKBc9Z3AqUqr_0B7HbW1unAv4qmt7-LJGUK_gsFBIaHISWJNt4n3yvmAnQNZHE-u=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/PAJ1XsOOI4fegwOo57ewAg", .shortcodes = {":socialdist:"}, .url = "https://yt3.ggpht.com/igBNi55-TACUi1xQkqMAor-IEXmt8He56K7pDTG5XoTsbM-rVswNzUfC5iwnfrpunWihrg=w48-h48-c-k-nd" },
    QtTubePlugin::Emoji { .representation = "UCkszU2WH9gy1mb0dV-11UJg/egJ1XufTKYfegwOo57ewAg", .shortcodes = {":shelterin:"}, .url = "https://yt3.ggpht.com/gjC5x98J4BoVSEPfFJaoLtc4tSBGSEdIlfL2FV4iJG9uGNykDP9oJC_QxAuBTJy6dakPxVeC=w48-h48-c-k-nd" }
};

std::pair<std::any, QtTubePlugin::ChannelData> getChannelData(const InnertubeEndpoints::ChannelResponse& response)
{
    std::any continuationData;
    QtTubePlugin::ChannelData data;

    if (const auto* c4Header = std::get_if<InnertubeObjects::ChannelC4Header>(&response.header))
        data.header = convertChannelHeader(*c4Header);
    else if (const auto* pageHeader = std::get_if<InnertubeObjects::ChannelPageHeader>(&response.header))
        data.header = convertChannelHeader(*pageHeader, response.mutations);

    if (response.contents.isArray()) // will be array in case of continuation
    {
        QtTubePlugin::ChannelTabData tabData;

        const QJsonArray contentsArr = response.contents.toArray();
        for (const QJsonValue& item : contentsArr)
        {
            if (const QJsonValue richItem = item["richItemRenderer"]; richItem.isObject())
            {
                const QJsonObject content = richItem["content"].toObject();
                QJsonObject::const_iterator it = content.begin();
                if (it.key() == "gridVideoRenderer" || it.key() == "videoRenderer")
                    tabData.items.append(convertVideo(InnertubeObjects::Video(it.value()), true));
                else if (it.key() == "reelItemRenderer")
                    tabData.items.append(convertVideo(InnertubeObjects::Reel(it.value()), true));
                else if (it.key() == "shortsLockupViewModel")
                    tabData.items.append(convertVideo(InnertubeObjects::ShortsLockupViewModel(it.value()), true));
            }
            else if (const QJsonValue continuation = item["continuationItemRenderer"]; continuation.isObject())
            {
                continuationData = continuation["continuationEndpoint"]["continuationCommand"]["token"].toString();
            }
        }

        data.tabs.append(tabData);
    }
    else if (response.contents.isObject())
    {
        const QJsonArray tabs = response.contents["twoColumnBrowseResultsRenderer"]["tabs"].toArray();
        for (qsizetype i = 0; i < tabs.size(); ++i)
            if (const QJsonValue tabRenderer = tabs[i]["tabRenderer"]; tabRenderer.isObject())
                data.tabs.append(convertTab(tabRenderer, continuationData));
    }

    return std::make_pair(continuationData, data);
}

QtTubePlugin::BrowseData getHistoryData(const InnertubeEndpoints::HistoryResponse& response)
{
    QtTubePlugin::BrowseData result;
    for (const InnertubeObjects::Video& video : response.videos)
        addVideo(result, video);
    return result;
}

QtTubePlugin::BrowseData getHomeData(const InnertubeEndpoints::HomeResponse& response)
{
    QtTubePlugin::BrowseData result;

    // non-authenticated users will be under the IOS_UNPLUGGED client,
    // which serves thumbnails in an odd aspect ratio.
    bool useThumbnailFromData = InnerTube::instance()->hasAuthenticated();

    for (const InnertubeEndpoints::HomeResponseItem& item : response.contents)
    {
        if (const auto* adSlot = std::get_if<InnertubeObjects::AdSlot>(&item))
            addVideo(result, *adSlot, useThumbnailFromData);
        else if (const auto* hrShelf = std::get_if<InnertubeObjects::HomeRichShelf>(&item))
            addShelf(result, convertShelf(*hrShelf, useThumbnailFromData));
        else if (const auto* hShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
            addShelf(result, convertShelf(*hShelf, useThumbnailFromData));
        else if (const auto* lockup = std::get_if<InnertubeObjects::LockupViewModel>(&item))
            addVideo(result, *lockup, useThumbnailFromData);
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
            addVideo(result, *video, useThumbnailFromData);
    }

    return result;
}

QtTubePlugin::LiveChat getLiveChatData(const InnertubeEndpoints::GetLiveChat& endpoint, QJsonValue& sendEndpoint, int& sentMessages)
{
    QtTubePlugin::LiveChat data;

    if (const QJsonValue actionPanel = endpoint.liveChatContinuation["actionPanel"]; actionPanel.isObject())
    {
        sendEndpoint = actionPanel["liveChatMessageInputRenderer"]["sendButton"]["buttonRenderer"]["serviceEndpoint"]["sendLiveChatMessageEndpoint"];
        sentMessages = 0;
        if (const QJsonValue restricted = actionPanel["liveChatRestrictedParticipationRenderer"]; restricted.isObject())
            data.restrictedMessage = InnertubeObjects::InnertubeString(restricted["message"]).text;
    }

    if (const QJsonValue viewSelector = endpoint.liveChatContinuation["header"]["liveChatHeaderRenderer"]["viewSelector"]; viewSelector.isObject())
    {
        const QJsonValue sortFilter = viewSelector["sortFilterSubMenuRenderer"]["subMenuItems"];
        auto continuationFor = [](const QJsonValue& obj) { return obj["continuation"]["reloadContinuationData"]["continuation"].toString(); };
        data.viewOptions.emplaceBack("Top chat", continuationFor(sortFilter[0]));
        data.viewOptions.emplaceBack("Live chat", continuationFor(sortFilter[1]));
    }

    const QJsonArray actions = endpoint.liveChatContinuation["actions"].toArray();
    for (const QJsonValue& action : actions)
        if (const QJsonValue& item = action["addChatItemAction"]["item"]; item.isObject())
            data.items.append(convertLiveChatItem(item));

    const QString continuation = endpoint.liveChatContinuation["continuations"][0]["invalidationContinuationData"]["continuation"].toString();
    if (!continuation.isEmpty())
        data.nextData = continuation;

    return data;
}

QtTubePlugin::LiveChatReplay getLiveChatReplayData(const InnertubeEndpoints::GetLiveChatReplay& endpoint)
{
    QtTubePlugin::LiveChatReplay data;

    const QJsonArray actions = endpoint.liveChatContinuation["actions"].toArray();
    for (const QJsonValue& action : actions)
    {
        if (const QJsonValue replayAction = action["replayChatItemAction"]; replayAction.isObject())
        {
            qint64 videoOffsetMs = replayAction["videoOffsetTimeMsec"].toString().toLongLong();
            const QJsonArray itemActions = replayAction["actions"].toArray();
            for (const QJsonValue& itemAction : itemActions)
                if (const QJsonValue item = itemAction["addChatItemAction"]["item"]; item.isObject())
                    data.items.emplaceBack(convertLiveChatItem(item), videoOffsetMs);
        }
    }

    const QString continuation = endpoint.liveChatContinuation["continuations"][0]["liveChatReplayContinuationData"]["continuation"].toString();
    if (!continuation.isEmpty())
        data.nextData = continuation;

    const QString seekContinuation = endpoint.liveChatContinuation["continuations"][1]["playerSeekContinuationData"]["continuation"].toString();
    if (!seekContinuation.isEmpty())
        data.seekData = seekContinuation;

    return data;
}

void getNextData(QtTubePlugin::VideoData& data, const InnertubeEndpoints::NextResponse& response)
{
    const InnertubeObjects::VideoPrimaryInfo& primaryInfo = response.contents.results.primaryInfo;
    const InnertubeObjects::VideoSecondaryInfo& secondaryInfo = response.contents.results.secondaryInfo;

    data.channel = convertChannel(secondaryInfo.owner, secondaryInfo.subscribeButton);
    data.viewCountText = g_settings->condensedCounts && !primaryInfo.viewCount.isLive
        ? primaryInfo.viewCount.extraShortViewCount.text + " views"
        : primaryInfo.viewCount.viewCount.text;
    data.videoId = response.videoId;

    const InnertubeObjects::LikeDislikeViewModel& likeDislikeViewModel = primaryInfo.videoActions.segmentedLikeDislikeButtonViewModel;
    const QString& likeStatus = likeDislikeViewModel.likeButtonViewModel.likeStatus;

    if (likeStatus == "LIKE")
        data.likeStatus = QtTubePlugin::VideoData::LikeStatus::Liked;
    else if (likeStatus == "DISLIKE")
        data.likeStatus = QtTubePlugin::VideoData::LikeStatus::Disliked;

    const InnertubeObjects::ButtonViewModel& likeViewModel = likeDislikeViewModel.likeButtonViewModel.toggleButtonViewModel.defaultButtonViewModel;
    const InnertubeObjects::ButtonViewModel& toggledLikeViewModel = likeDislikeViewModel.likeButtonViewModel.toggleButtonViewModel.toggledButtonViewModel;

    QString fullLikeCount = StringUtils::extractDigits(likeViewModel.accessibilityText);
    data.likeCountText = g_settings->condensedCounts ? likeViewModel.title : fullLikeCount;

    const QJsonValue defaultLikeEndpoint = likeViewModel.onTap["serialCommand"]["commands"][1]["innertubeCommand"]["likeEndpoint"];
    const QJsonValue toggledLikeEndpoint = toggledLikeViewModel.onTap["serialCommand"]["commands"][1]["innertubeCommand"]["likeEndpoint"];

    data.likeData.like = defaultLikeEndpoint["likeParams"].toString();
    data.likeData.removeLike = toggledLikeEndpoint["removeLikeParams"].toString();
    data.likeData.dislike = defaultLikeEndpoint["likeParams"].toString();
    data.likeData.removeDislike = toggledLikeEndpoint["removeLikeParams"].toString();

    if (g_settings->returnDislikes)
    {
        HttpReply* reply = HttpRequest().get("https://returnyoutubedislikeapi.com/votes?videoId=" + data.videoId);

        QEventLoop loop;
        QObject::connect(reply, &HttpReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->isSuccessful())
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            qint64 dislikes = doc["dislikes"].toVariant().toLongLong();
            qint64 likes = QLocale::system().toLongLong(fullLikeCount);
            data.likeDislikeRatio = double(likes) / (likes + dislikes);

        #ifdef QTTUBE_HAS_ICU
            if (g_settings->condensedCounts)
                data.dislikeCountText = StringUtils::condensedNumericString(dislikes);
            else
        #endif
                data.dislikeCountText = QLocale::system().toString(dislikes);
        }
        else
        {
            data.dislikeCountText = "Dislike";
        }
    }
    else
    {
        data.dislikeCountText = "Dislike";
    }

    data.dateText = primaryInfo.dateText.text;
    if (!primaryInfo.superTitleLink.text.isEmpty())
        data.dateText += " | " + primaryInfo.superTitleLink.toRichText(true);

    data.descriptionText = TubeUtils::unattribute(secondaryInfo.attributedDescription).toRichText(false);

    if (const QString& commentsContinuation = response.contents.results.commentsSectionContinuation; !commentsContinuation.isEmpty())
        data.continuations.comments = commentsContinuation;
    if (const QString& feedContinuation = response.contents.secondaryResults.feedContinuation; !feedContinuation.isEmpty())
        data.continuations.recommended = feedContinuation;

    if (const std::optional<InnertubeObjects::LiveChat>& conversationBar = response.contents.conversationBar)
    {
        data.initialLiveChatData = QtTubePlugin::InitialLiveChatData {
            .data = conversationBar->continuations.front(),
            .isReplay = conversationBar->isReplay,
            .platformEmojis = g_platformEmojis
        };
    }

    for (const InnertubeObjects::WatchNextFeedItem& item : response.contents.secondaryResults.feed)
        std::visit([&data](auto&& v) { addVideo(data.recommendedVideos, v); }, item);
}

QtTubePlugin::NotificationsData getNotificationsData(const InnertubeEndpoints::NotificationMenuResponse& response)
{
    QtTubePlugin::NotificationsData result;
    for (const InnertubeObjects::Notification& notification : response.notifications)
        addNotification(result, convertNotification(notification));
    return result;
}

void getPlayerData(QtTubePlugin::VideoData& data, const InnertubeEndpoints::PlayerResponse& response)
{
    data.isLiveContent = response.videoDetails.isLive || response.videoDetails.isUpcoming;
    data.sourceMetadata = &g_metadata;
    data.titleText = response.videoDetails.title;
    data.videoUrlPrefix = "https://www.youtube.com/watch?v=";
}

QtTubePlugin::BrowseData getSearchData(const InnertubeEndpoints::SearchResponse& response)
{
    QtTubePlugin::BrowseData result;
    for (const InnertubeEndpoints::SearchResponseItem& item : response.contents)
    {
        if (const auto* channel = std::get_if<InnertubeObjects::Channel>(&item))
            addChannel(result, convertChannel(*channel));
        else if (const auto* rShelf = std::get_if<InnertubeObjects::ReelShelf>(&item); rShelf && !g_settings->hideSearchShelves)
            addShelf(result, convertShelf(*rShelf));
        else if (const auto* vShelf = std::get_if<InnertubeObjects::VerticalVideoShelf>(&item); vShelf && !g_settings->hideSearchShelves)
            addShelf(result, convertShelf(*vShelf));
        else if (const auto* video = std::get_if<InnertubeObjects::Video>(&item))
            addVideo(result, *video);
    }
    return result;
}

QtTubePlugin::BrowseData getSubscriptionsData(const InnertubeEndpoints::SubscriptionsResponse& response)
{
    QtTubePlugin::BrowseData result;
    for (const InnertubeObjects::Video& video : response.videos)
        addVideo(result, video);
    return result;
}

QtTubePlugin::BrowseData getTrendingData(const InnertubeEndpoints::TrendingResponse& response)
{
    QtTubePlugin::BrowseData result;

    for (const InnertubeEndpoints::TrendingResponseItem& item : response.contents)
    {
        if (const auto* hShelf = std::get_if<InnertubeObjects::HorizontalVideoShelf>(&item))
            addShelf(result, convertShelf(*hShelf));
        else if (const auto* rShelf = std::get_if<InnertubeObjects::ReelShelf>(&item))
            addShelf(result, convertShelf(*rShelf));
        else if (const auto* sShelf = std::get_if<InnertubeObjects::StandardVideoShelf>(&item))
            addShelf(result, convertShelf(*sShelf));
    }

    return result;
}
