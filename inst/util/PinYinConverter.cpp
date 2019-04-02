#include <QDebug>
#include <QByteArray>
#include "PinYinConverter.h"


const qint16 PinyinConveter::oneLevelChCode[] = {
	-20319, -20317, -20304, -20295, -20292, -20283, -20265, -20257, -20242,

	-20230, -20051, -20036, -20032, -20026, -20002, -19990, -19986, -19982,

	-19976, -19805, -19784, -19775, -19774, -19763, -19756, -19751, -19746,

	-19741, -19739, -19728, -19725, -19715, -19540, -19531, -19525, -19515,

	-19500, -19484, -19479, -19467, -19289, -19288, -19281, -19275, -19270,

	-19263, -19261, -19249, -19243, -19242, -19238, -19235, -19227, -19224,

	-19218, -19212, -19038, -19023, -19018, -19006, -19003, -18996, -18977,

	-18961, -18952, -18783, -18774, -18773, -18763, -18756, -18741, -18735,

	-18731, -18722, -18710, -18697, -18696, -18526, -18518, -18501, -18490,

	-18478, -18463, -18448, -18447, -18446, -18239, -18237, -18231, -18220,

	-18211, -18201, -18184, -18183, -18181, -18012, -17997, -17988, -17970,

	-17964, -17961, -17950, -17947, -17931, -17928, -17922, -17759, -17752,

	-17733, -17730, -17721, -17703, -17701, -17697, -17692, -17683, -17676,

	-17496, -17487, -17482, -17468, -17454, -17433, -17427, -17417, -17202,

	-17185, -16983, -16970, -16942, -16915, -16733, -16708, -16706, -16689,

	-16664, -16657, -16647, -16474, -16470, -16465, -16459, -16452, -16448,

	-16433, -16429, -16427, -16423, -16419, -16412, -16407, -16403, -16401,

	-16393, -16220, -16216, -16212, -16205, -16202, -16187, -16180, -16171,

	-16169, -16158, -16155, -15959, -15958, -15944, -15933, -15920, -15915,

	-15903, -15889, -15878, -15707, -15701, -15681, -15667, -15661, -15659,

	-15652, -15640, -15631, -15625, -15454, -15448, -15436, -15435, -15419,

	-15416, -15408, -15394, -15385, -15377, -15375, -15369, -15363, -15362,

	-15183, -15180, -15165, -15158, -15153, -15150, -15149, -15144, -15143,

	-15141, -15140, -15139, -15128, -15121, -15119, -15117, -15110, -15109,

	-14941, -14937, -14933, -14930, -14929, -14928, -14926, -14922, -14921,

	-14914, -14908, -14902, -14894, -14889, -14882, -14873, -14871, -14857,

	-14678, -14674, -14670, -14668, -14663, -14654, -14645, -14630, -14594,

	-14429, -14407, -14399, -14384, -14379, -14368, -14355, -14353, -14345,

	-14170, -14159, -14151, -14149, -14145, -14140, -14137, -14135, -14125,

	-14123, -14122, -14112, -14109, -14099, -14097, -14094, -14092, -14090,

	-14087, -14083, -13917, -13914, -13910, -13907, -13906, -13905, -13896,

	-13894, -13878, -13870, -13859, -13847, -13831, -13658, -13611, -13601,

	-13406, -13404, -13400, -13398, -13395, -13391, -13387, -13383, -13367,

	-13359, -13356, -13343, -13340, -13329, -13326, -13318, -13147, -13138,

	-13120, -13107, -13096, -13095, -13091, -13076, -13068, -13063, -13060,

	-12888, -12875, -12871, -12860, -12858, -12852, -12849, -12838, -12831,

	-12829, -12812, -12802, -12607, -12597, -12594, -12585, -12556, -12359,

	-12346, -12320, -12300, -12120, -12099, -12089, -12074, -12067, -12058,

	-12039, -11867, -11861, -11847, -11831, -11798, -11781, -11604, -11589,

	-11536, -11358, -11340, -11339, -11324, -11303, -11097, -11077, -11067,

	-11055, -11052, -11045, -11041, -11038, -11024, -11020, -11019, -11018,

	-11014, -10838, -10832, -10815, -10800, -10790, -10780, -10764, -10587,

	-10544, -10533, -10519, -10331, -10329, -10328, -10322, -10315, -10309,

	-10307, -10296, -10281, -10274, -10270, -10262, -10260, -10256, -10254

};

const QString PinyinConveter::oneLevelPyValue[] = {
	"A", "Ai", "An", "Ang", "Ao", "Ba", "Bai", "Ban", "Bang", "Bao", "Bei",

	"Ben", "Beng", "Bi", "Bian", "Biao", "Bie", "Bin", "Bing", "Bo", "Bu",

	"Ba", "Cai", "Can", "Cang", "Cao", "Ce", "Ceng", "Cha", "Chai", "Chan",

	"Chang", "Chao", "Che", "Chen", "Cheng", "Chi", "Chong", "Chou", "Chu",

	"Chuai", "Chuan", "Chuang", "Chui", "Chun", "Chuo", "Ci", "Cong", "Cou",

	"Cu", "Cuan", "Cui", "Cun", "Cuo", "Da", "Dai", "Dan", "Dang", "Dao", "De",

	"Deng", "Di", "Dian", "Diao", "Die", "Ding", "Diu", "Dong", "Dou", "Du",

	"Duan", "Dui", "Dun", "Duo", "E", "En", "Er", "Fa", "Fan", "Fang", "Fei",

	"Fen", "Feng", "Fo", "Fou", "Fu", "Ga", "Gai", "Gan", "Gang", "Gao", "Ge",

	"Gei", "Gen", "Geng", "Gong", "Gou", "Gu", "Gua", "Guai", "Guan", "Guang",

	"Gui", "Gun", "Guo", "Ha", "Hai", "Han", "Hang", "Hao", "He", "Hei", "Hen",

	"Heng", "Hong", "Hou", "Hu", "Hua", "Huai", "Huan", "Huang", "Hui", "Hun",

	"Huo", "Ji", "Jia", "Jian", "Jiang", "Jiao", "Jie", "Jin", "Jing", "Jiong",

	"Jiu", "Ju", "Juan", "Jue", "Jun", "Ka", "Kai", "Kan", "Kang", "Kao", "Ke",

	"Ken", "Keng", "Kong", "Kou", "Ku", "Kua", "Kuai", "Kuan", "Kuang", "Kui",

	"Kun", "Kuo", "La", "Lai", "Lan", "Lang", "Lao", "Le", "Lei", "Leng", "Li",

	"Lia", "Lian", "Liang", "Liao", "Lie", "Lin", "Ling", "Liu", "Long", "Lou",

	"Lu", "Lv", "Luan", "Lue", "Lun", "Luo", "Ma", "Mai", "Man", "Mang", "Mao",

	"Me", "Mei", "Men", "Meng", "Mi", "Mian", "Miao", "Mie", "Min", "Ming", "Miu",

	"Mo", "Mou", "Mu", "Na", "Nai", "Nan", "Nang", "Nao", "Ne", "Nei", "Nen",

	"Neng", "Ni", "Nian", "Niang", "Niao", "Nie", "Nin", "Ning", "Niu", "Nong",

	"Nu", "Nv", "Nuan", "Nue", "Nuo", "O", "Ou", "Pa", "Pai", "Pan", "Pang",

	"Pao", "Pei", "Pen", "Peng", "Pi", "Pian", "Piao", "Pie", "Pin", "Ping",

	"Po", "Pu", "Qi", "Qia", "Qian", "Qiang", "Qiao", "Qie", "Qin", "Qing",

	"Qiong", "Qiu", "Qu", "Quan", "Que", "Qun", "Ran", "Rang", "Rao", "Re",

	"Ren", "Reng", "Ri", "Rong", "Rou", "Ru", "Ruan", "Rui", "Run", "Ruo",

	"Sa", "Sai", "San", "Sang", "Sao", "Se", "Sen", "Seng", "Sha", "Shai",

	"Shan", "Shang", "Shao", "She", "Shen", "Sheng", "Shi", "Shou", "Shu",

	"Shua", "Shuai", "Shuan", "Shuang", "Shui", "Shun", "Shuo", "Si", "Song",

	"Sou", "Su", "Suan", "Sui", "Sun", "Suo", "Ta", "Tai", "Tan", "Tang",

	"Tao", "Te", "Teng", "Ti", "Tian", "Tiao", "Tie", "Ting", "Tong", "Tou",

	"Tu", "Tuan", "Tui", "Tun", "Tuo", "Wa", "Wai", "Wan", "Wang", "Wei",

	"Wen", "Weng", "Wo", "Wu", "Xi", "Xia", "Xian", "Xiang", "Xiao", "Xie",

	"Xin", "Xing", "Xiong", "Xiu", "Xu", "Xuan", "Xue", "Xun", "Ya", "Yan",

	"Yang", "Yao", "Ye", "Yi", "Yin", "Ying", "Yo", "Yong", "You", "Yu",

	"Yuan", "Yue", "Yun", "Za", "Zai", "Zan", "Zang", "Zao", "Ze", "Zei",

	"Zen", "Zeng", "Zha", "Zhai", "Zhan", "Zhang","Zhao", "Zhe", "Zhen",

	"Zheng", "Zhi", "Zhong", "Zhou", "Zhu", "Zhua", "Zhuai", "Zhuan",

	"Zhuang", "Zhui", "Zhun", "Zhuo", "Zi", "Zong", "Zou", "Zu", "Zuan",

	"Zui", "Zun", "Zuo"
};

const QString PinyinConveter::twoLevelPyValue[] =
{
	"Chu","Ji","Wu","Gai","Nian","Sa","Pi","Gen","Cheng","Ge","Nao","E","Shu","Yu","Pie","Bi",
	"Tuo","Yao","Yao","Zhi","Di","Xin","Yin","Kui","Yu","Gao","Tao","Dian","Ji","Nai","Nie","Ji",
	"Qi","Mi","Bei","Se","Gu","Ze","She","Cuo","Yan","Jue","Si","Ye","Yan","Fang","Po","Gui",
	"Kui","Bian","Ze","Gua","You","Ce","Yi","Wen","Jing","Ku","Gui","Kai","La","Ji","Yan","Wan",
	"Kuai","Piao","Jue","Qiao","Huo","Yi","Tong","Wang","Dan","Ding","Zhang","Le","Sa","Yi","Mu","Ren",
	"Yu","Pi","Ya","Wa","Wu","Chang","Cang","Kang","Zhu","Ning","Ka","You","Yi","Gou","Tong","Tuo",
	"Ni","Ga","Ji","Er","You","Kua","Kan","Zhu","Yi","Tiao","Chai","Jiao","Nong","Mou","Chou","Yan",
	"Li","Qiu","Li","Yu","Ping","Yong","Si","Feng","Qian","Ruo","Pai","Zhuo","Shu","Luo","Wo","Bi",
	"Ti","Guan","Kong","Ju","Fen","Yan","Xie","Ji","Wei","Zong","Lou","Tang","Bin","Nuo","Chi","Xi",
	"Jing","Jian","Jiao","Jiu","Tong","Xuan","Dan","Tong","Tun","She","Qian","Zu","Yue","Cuan","Di","Xi",
	"Xun","Hong","Guo","Chan","Kui","Bao","Pu","Hong","Fu","Fu","Su","Si","Wen","Yan","Bo","Gun",
	"Mao","Xie","Luan","Pou","Bing","Ying","Luo","Lei","Liang","Hu","Lie","Xian","Song","Ping","Zhong","Ming",
	"Yan","Jie","Hong","Shan","Ou","Ju","Ne","Gu","He","Di","Zhao","Qu","Dai","Kuang","Lei","Gua",
	"Jie","Hui","Shen","Gou","Quan","Zheng","Hun","Xu","Qiao","Gao","Kuang","Ei","Zou","Zhuo","Wei","Yu",
	"Shen","Chan","Sui","Chen","Jian","Xue","Ye","E","Yu","Xuan","An","Di","Zi","Pian","Mo","Dang",
	"Su","Shi","Mi","Zhe","Jian","Zen","Qiao","Jue","Yan","Zhan","Chen","Dan","Jin","Zuo","Wu","Qian",
	"Jing","Ban","Yan","Zuo","Bei","Jing","Gai","Zhi","Nie","Zou","Chui","Pi","Wei","Huang","Wei","Xi",
	"Han","Qiong","Kuang","Mang","Wu","Fang","Bing","Pi","Bei","Ye","Di","Tai","Jia","Zhi","Zhu","Kuai",
	"Qie","Xun","Yun","Li","Ying","Gao","Xi","Fu","Pi","Tan","Yan","Juan","Yan","Yin","Zhang","Po",
	"Shan","Zou","Ling","Feng","Chu","Huan","Mai","Qu","Shao","He","Ge","Meng","Xu","Xie","Sou","Xie",
	"Jue","Jian","Qian","Dang","Chang","Si","Bian","Ben","Qiu","Ben","E","Fa","Shu","Ji","Yong","He",
	"Wei","Wu","Ge","Zhen","Kuang","Pi","Yi","Li","Qi","Ban","Gan","Long","Dian","Lu","Che","Di",
	"Tuo","Ni","Mu","Ao","Ya","Die","Dong","Kai","Shan","Shang","Nao","Gai","Yin","Cheng","Shi","Guo",
	"Xun","Lie","Yuan","Zhi","An","Yi","Pi","Nian","Peng","Tu","Sao","Dai","Ku","Die","Yin","Leng",
	"Hou","Ge","Yuan","Man","Yong","Liang","Chi","Xin","Pi","Yi","Cao","Jiao","Nai","Du","Qian","Ji",
	"Wan","Xiong","Qi","Xiang","Fu","Yuan","Yun","Fei","Ji","Li","E","Ju","Pi","Zhi","Rui","Xian",
	"Chang","Cong","Qin","Wu","Qian","Qi","Shan","Bian","Zhu","Kou","Yi","Mo","Gan","Pie","Long","Ba",
	"Mu","Ju","Ran","Qing","Chi","Fu","Ling","Niao","Yin","Mao","Ying","Qiong","Min","Tiao","Qian","Yi",
	"Rao","Bi","Zi","Ju","Tong","Hui","Zhu","Ting","Qiao","Fu","Ren","Xing","Quan","Hui","Xun","Ming",
	"Qi","Jiao","Chong","Jiang","Luo","Ying","Qian","Gen","Jin","Mai","Sun","Hong","Zhou","Kan","Bi","Shi",
	"Wo","You","E","Mei","You","Li","Tu","Xian","Fu","Sui","You","Di","Shen","Guan","Lang","Ying",
	"Chun","Jing","Qi","Xi","Song","Jin","Nai","Qi","Ba","Shu","Chang","Tie","Yu","Huan","Bi","Fu",
	"Tu","Dan","Cui","Yan","Zu","Dang","Jian","Wan","Ying","Gu","Han","Qia","Feng","Shen","Xiang","Wei",
	"Chan","Kai","Qi","Kui","Xi","E","Bao","Pa","Ting","Lou","Pai","Xuan","Jia","Zhen","Shi","Ru",
	"Mo","En","Bei","Weng","Hao","Ji","Li","Bang","Jian","Shuo","Lang","Ying","Yu","Su","Meng","Dou",
	"Xi","Lian","Cu","Lin","Qu","Kou","Xu","Liao","Hui","Xun","Jue","Rui","Zui","Ji","Meng","Fan",
	"Qi","Hong","Xie","Hong","Wei","Yi","Weng","Sou","Bi","Hao","Tai","Ru","Xun","Xian","Gao","Li",
	"Huo","Qu","Heng","Fan","Nie","Mi","Gong","Yi","Kuang","Lian","Da","Yi","Xi","Zang","Pao","You",
	"Liao","Ga","Gan","Ti","Men","Tuan","Chen","Fu","Pin","Niu","Jie","Jiao","Za","Yi","Lv","Jun",
	"Tian","Ye","Ai","Na","Ji","Guo","Bai","Ju","Pou","Lie","Qian","Guan","Die","Zha","Ya","Qin",
	"Yu","An","Xuan","Bing","Kui","Yuan","Shu","En","Chuai","Jian","Shuo","Zhan","Nuo","Sang","Luo","Ying",
	"Zhi","Han","Zhe","Xie","Lu","Zun","Cuan","Gan","Huan","Pi","Xing","Zhuo","Huo","Zuan","Nang","Yi",
	"Te","Dai","Shi","Bu","Chi","Ji","Kou","Dao","Le","Zha","A","Yao","Fu","Mu","Yi","Tai",
	"Li","E","Bi","Bei","Guo","Qin","Yin","Za","Ka","Ga","Gua","Ling","Dong","Ning","Duo","Nao",
	"You","Si","Kuang","Ji","Shen","Hui","Da","Lie","Yi","Xiao","Bi","Ci","Guang","Yue","Xiu","Yi",
	"Pai","Kuai","Duo","Ji","Mie","Mi","Zha","Nong","Gen","Mou","Mai","Chi","Lao","Geng","En","Zha",
	"Suo","Zao","Xi","Zuo","Ji","Feng","Ze","Nuo","Miao","Lin","Zhuan","Zhou","Tao","Hu","Cui","Sha",
	"Yo","Dan","Bo","Ding","Lang","Li","Shua","Chuo","Die","Da","Nan","Li","Kui","Jie","Yong","Kui",
	"Jiu","Sou","Yin","Chi","Jie","Lou","Ku","Wo","Hui","Qin","Ao","Su","Du","Ke","Nie","He",
	"Chen","Suo","Ge","A","En","Hao","Dia","Ai","Ai","Suo","Hei","Tong","Chi","Pei","Lei","Cao",
	"Piao","Qi","Ying","Beng","Sou","Di","Mi","Peng","Jue","Liao","Pu","Chuai","Jiao","O","Qin","Lu",
	"Ceng","Deng","Hao","Jin","Jue","Yi","Sai","Pi","Ru","Cha","Huo","Nang","Wei","Jian","Nan","Lun",
	"Hu","Ling","You","Yu","Qing","Yu","Huan","Wei","Zhi","Pei","Tang","Dao","Ze","Guo","Wei","Wo",
	"Man","Zhang","Fu","Fan","Ji","Qi","Qian","Qi","Qu","Ya","Xian","Ao","Cen","Lan","Ba","Hu",
	"Ke","Dong","Jia","Xiu","Dai","Gou","Mao","Min","Yi","Dong","Qiao","Xun","Zheng","Lao","Lai","Song",
	"Yan","Gu","Xiao","Guo","Kong","Jue","Rong","Yao","Wai","Zai","Wei","Yu","Cuo","Lou","Zi","Mei",
	"Sheng","Song","Ji","Zhang","Lin","Deng","Bin","Yi","Dian","Chi","Pang","Cu","Xun","Yang","Hou","Lai",
	"Xi","Chang","Huang","Yao","Zheng","Jiao","Qu","San","Fan","Qiu","An","Guang","Ma","Niu","Yun","Xia",
	"Pao","Fei","Rong","Kuai","Shou","Sun","Bi","Juan","Li","Yu","Xian","Yin","Suan","Yi","Guo","Luo",
	"Ni","She","Cu","Mi","Hu","Cha","Wei","Wei","Mei","Nao","Zhang","Jing","Jue","Liao","Xie","Xun",
	"Huan","Chuan","Huo","Sun","Yin","Dong","Shi","Tang","Tun","Xi","Ren","Yu","Chi","Yi","Xiang","Bo",
	"Yu","Hun","Zha","Sou","Mo","Xiu","Jin","San","Zhuan","Nang","Pi","Wu","Gui","Pao","Xiu","Xiang",
	"Tuo","An","Yu","Bi","Geng","Ao","Jin","Chan","Xie","Lin","Ying","Shu","Dao","Cun","Chan","Wu",
	"Zhi","Ou","Chong","Wu","Kai","Chang","Chuang","Song","Bian","Niu","Hu","Chu","Peng","Da","Yang","Zuo",
	"Ni","Fu","Chao","Yi","Yi","Tong","Yan","Ce","Kai","Xun","Ke","Yun","Bei","Song","Qian","Kui",
	"Kun","Yi","Ti","Quan","Qie","Xing","Fei","Chang","Wang","Chou","Hu","Cui","Yun","Kui","E","Leng",
	"Zhui","Qiao","Bi","Su","Qie","Yong","Jing","Qiao","Chong","Chu","Lin","Meng","Tian","Hui","Shuan","Yan",
	"Wei","Hong","Min","Kang","Ta","Lv","Kun","Jiu","Lang","Yu","Chang","Xi","Wen","Hun","E","Qu",
	"Que","He","Tian","Que","Kan","Jiang","Pan","Qiang","San","Qi","Si","Cha","Feng","Yuan","Mu","Mian",
	"Dun","Mi","Gu","Bian","Wen","Hang","Wei","Le","Gan","Shu","Long","Lu","Yang","Si","Duo","Ling",
	"Mao","Luo","Xuan","Pan","Duo","Hong","Min","Jing","Huan","Wei","Lie","Jia","Zhen","Yin","Hui","Zhu",
	"Ji","Xu","Hui","Tao","Xun","Jiang","Liu","Hu","Xun","Ru","Su","Wu","Lai","Wei","Zhuo","Juan",
	"Cen","Bang","Xi","Mei","Huan","Zhu","Qi","Xi","Song","Du","Zhuo","Pei","Mian","Gan","Fei","Cong",
	"Shen","Guan","Lu","Shuan","Xie","Yan","Mian","Qiu","Sou","Huang","Xu","Pen","Jian","Xuan","Wo","Mei",
	"Yan","Qin","Ke","She","Mang","Ying","Pu","Li","Ru","Ta","Hun","Bi","Xiu","Fu","Tang","Pang",
	"Ming","Huang","Ying","Xiao","Lan","Cao","Hu","Luo","Huan","Lian","Zhu","Yi","Lu","Xuan","Gan","Shu",
	"Si","Shan","Shao","Tong","Chan","Lai","Sui","Li","Dan","Chan","Lian","Ru","Pu","Bi","Hao","Zhuo",
	"Han","Xie","Ying","Yue","Fen","Hao","Ba","Bao","Gui","Dang","Mi","You","Chen","Ning","Jian","Qian",
	"Wu","Liao","Qian","Huan","Jian","Jian","Zou","Ya","Wu","Jiong","Ze","Yi","Er","Jia","Jing","Dai",
	"Hou","Pang","Bu","Li","Qiu","Xiao","Ti","Qun","Kui","Wei","Huan","Lu","Chuan","Huang","Qiu","Xia",
	"Ao","Gou","Ta","Liu","Xian","Lin","Ju","Xie","Miao","Sui","La","Ji","Hui","Tuan","Zhi","Kao",
	"Zhi","Ji","E","Chan","Xi","Ju","Chan","Jing","Nu","Mi","Fu","Bi","Yu","Che","Shuo","Fei",
	"Yan","Wu","Yu","Bi","Jin","Zi","Gui","Niu","Yu","Si","Da","Zhou","Shan","Qie","Ya","Rao",
	"Shu","Luan","Jiao","Pin","Cha","Li","Ping","Wa","Xian","Suo","Di","Wei","E","Jing","Biao","Jie",
	"Chang","Bi","Chan","Nu","Ao","Yuan","Ting","Wu","Gou","Mo","Pi","Ai","Pin","Chi","Li","Yan",
	"Qiang","Piao","Chang","Lei","Zhang","Xi","Shan","Bi","Niao","Mo","Shuang","Ga","Ga","Fu","Nu","Zi",
	"Jie","Jue","Bao","Zang","Si","Fu","Zou","Yi","Nu","Dai","Xiao","Hua","Pian","Li","Qi","Ke",
	"Zhui","Can","Zhi","Wu","Ao","Liu","Shan","Biao","Cong","Chan","Ji","Xiang","Jiao","Yu","Zhou","Ge",
	"Wan","Kuang","Yun","Pi","Shu","Gan","Xie","Fu","Zhou","Fu","Chu","Dai","Ku","Hang","Jiang","Geng",
	"Xiao","Ti","Ling","Qi","Fei","Shang","Gun","Duo","Shou","Liu","Quan","Wan","Zi","Ke","Xiang","Ti",
	"Miao","Hui","Si","Bian","Gou","Zhui","Min","Jin","Zhen","Ru","Gao","Li","Yi","Jian","Bin","Piao",
	"Man","Lei","Miao","Sao","Xie","Liao","Zeng","Jiang","Qian","Qiao","Huan","Zuan","Yao","Ji","Chuan","Zai",
	"Yong","Ding","Ji","Wei","Bin","Min","Jue","Ke","Long","Dian","Dai","Po","Min","Jia","Er","Gong",
	"Xu","Ya","Heng","Yao","Luo","Xi","Hui","Lian","Qi","Ying","Qi","Hu","Kun","Yan","Cong","Wan",
	"Chen","Ju","Mao","Yu","Yuan","Xia","Nao","Ai","Tang","Jin","Huang","Ying","Cui","Cong","Xuan","Zhang",
	"Pu","Can","Qu","Lu","Bi","Zan","Wen","Wei","Yun","Tao","Wu","Shao","Qi","Cha","Ma","Li",
	"Pi","Miao","Yao","Rui","Jian","Chu","Cheng","Cong","Xiao","Fang","Pa","Zhu","Nai","Zhi","Zhe","Long",
	"Jiu","Ping","Lu","Xia","Xiao","You","Zhi","Tuo","Zhi","Ling","Gou","Di","Li","Tuo","Cheng","Kao",
	"Lao","Ya","Rao","Zhi","Zhen","Guang","Qi","Ting","Gua","Jiu","Hua","Heng","Gui","Jie","Luan","Juan",
	"An","Xu","Fan","Gu","Fu","Jue","Zi","Suo","Ling","Chu","Fen","Du","Qian","Zhao","Luo","Chui",
	"Liang","Guo","Jian","Di","Ju","Cou","Zhen","Nan","Zha","Lian","Lan","Ji","Pin","Ju","Qiu","Duan",
	"Chui","Chen","Lv","Cha","Ju","Xuan","Mei","Ying","Zhen","Fei","Ta","Sun","Xie","Gao","Cui","Gao",
	"Shuo","Bin","Rong","Zhu","Xie","Jin","Qiang","Qi","Chu","Tang","Zhu","Hu","Gan","Yue","Qing","Tuo",
	"Jue","Qiao","Qin","Lu","Zun","Xi","Ju","Yuan","Lei","Yan","Lin","Bo","Cha","You","Ao","Mo",
	"Cu","Shang","Tian","Yun","Lian","Piao","Dan","Ji","Bin","Yi","Ren","E","Gu","Ke","Lu","Zhi",
	"Yi","Zhen","Hu","Li","Yao","Shi","Zhi","Quan","Lu","Zhe","Nian","Wang","Chuo","Zi","Cou","Lu",
	"Lin","Wei","Jian","Qiang","Jia","Ji","Ji","Kan","Deng","Gai","Jian","Zang","Ou","Ling","Bu","Beng",
	"Zeng","Pi","Po","Ga","La","Gan","Hao","Tan","Gao","Ze","Xin","Yun","Gui","He","Zan","Mao",
	"Yu","Chang","Ni","Qi","Sheng","Ye","Chao","Yan","Hui","Bu","Han","Gui","Xuan","Kui","Ai","Ming",
	"Tun","Xun","Yao","Xi","Nang","Ben","Shi","Kuang","Yi","Zhi","Zi","Gai","Jin","Zhen","Lai","Qiu",
	"Ji","Dan","Fu","Chan","Ji","Xi","Di","Yu","Gou","Jin","Qu","Jian","Jiang","Pin","Mao","Gu",
	"Wu","Gu","Ji","Ju","Jian","Pian","Kao","Qie","Suo","Bai","Ge","Bo","Mao","Mu","Cui","Jian",
	"San","Shu","Chang","Lu","Pu","Qu","Pie","Dao","Xian","Chuan","Dong","Ya","Yin","Ke","Yun","Fan",
	"Chi","Jiao","Du","Die","You","Yuan","Guo","Yue","Wo","Rong","Huang","Jing","Ruan","Tai","Gong","Zhun",
	"Na","Yao","Qian","Long","Dong","Ka","Lu","Jia","Shen","Zhou","Zuo","Gua","Zhen","Qu","Zhi","Jing",
	"Guang","Dong","Yan","Kuai","Sa","Hai","Pian","Zhen","Mi","Tun","Luo","Cuo","Pao","Wan","Niao","Jing",
	"Yan","Fei","Yu","Zong","Ding","Jian","Cou","Nan","Mian","Wa","E","Shu","Cheng","Ying","Ge","Lv",
	"Bin","Teng","Zhi","Chuai","Gu","Meng","Sao","Shan","Lian","Lin","Yu","Xi","Qi","Sha","Xin","Xi",
	"Biao","Sa","Ju","Sou","Biao","Biao","Shu","Gou","Gu","Hu","Fei","Ji","Lan","Yu","Pei","Mao",
	"Zhan","Jing","Ni","Liu","Yi","Yang","Wei","Dun","Qiang","Shi","Hu","Zhu","Xuan","Tai","Ye","Yang",
	"Wu","Han","Men","Chao","Yan","Hu","Yu","Wei","Duan","Bao","Xuan","Bian","Tui","Liu","Man","Shang",
	"Yun","Yi","Yu","Fan","Sui","Xian","Jue","Cuan","Huo","Tao","Xu","Xi","Li","Hu","Jiong","Hu",
	"Fei","Shi","Si","Xian","Zhi","Qu","Hu","Fu","Zuo","Mi","Zhi","Ci","Zhen","Tiao","Qi","Chan",
	"Xi","Zhuo","Xi","Rang","Te","Tan","Dui","Jia","Hui","Nv","Nin","Yang","Zi","Que","Qian","Min",
	"Te","Qi","Dui","Mao","Men","Gang","Yu","Yu","Ta","Xue","Miao","Ji","Gan","Dang","Hua","Che",
	"Dun","Ya","Zhuo","Bian","Feng","Fa","Ai","Li","Long","Zha","Tong","Di","La","Tuo","Fu","Xing",
	"Mang","Xia","Qiao","Zhai","Dong","Nao","Ge","Wo","Qi","Dui","Bei","Ding","Chen","Zhou","Jie","Di",
	"Xuan","Bian","Zhe","Gun","Sang","Qing","Qu","Dun","Deng","Jiang","Ca","Meng","Bo","Kan","Zhi","Fu",
	"Fu","Xu","Mian","Kou","Dun","Miao","Dan","Sheng","Yuan","Yi","Sui","Zi","Chi","Mou","Lai","Jian",
	"Di","Suo","Ya","Ni","Sui","Pi","Rui","Sou","Kui","Mao","Ke","Ming","Piao","Cheng","Kan","Lin",
	"Gu","Ding","Bi","Quan","Tian","Fan","Zhen","She","Wan","Tuan","Fu","Gang","Gu","Li","Yan","Pi",
	"Lan","Li","Ji","Zeng","He","Guan","Juan","Jin","Ga","Yi","Po","Zhao","Liao","Tu","Chuan","Shan",
	"Men","Chai","Nv","Bu","Tai","Ju","Ban","Qian","Fang","Kang","Dou","Huo","Ba","Yu","Zheng","Gu",
	"Ke","Po","Bu","Bo","Yue","Mu","Tan","Dian","Shuo","Shi","Xuan","Ta","Bi","Ni","Pi","Duo",
	"Kao","Lao","Er","You","Cheng","Jia","Nao","Ye","Cheng","Diao","Yin","Kai","Zhu","Ding","Diu","Hua",
	"Quan","Ha","Sha","Diao","Zheng","Se","Chong","Tang","An","Ru","Lao","Lai","Te","Keng","Zeng","Li",
	"Gao","E","Cuo","Lve","Liu","Kai","Jian","Lang","Qin","Ju","A","Qiang","Nuo","Ben","De","Ke",
	"Kun","Gu","Huo","Pei","Juan","Tan","Zi","Qie","Kai","Si","E","Cha","Sou","Huan","Ai","Lou",
	"Qiang","Fei","Mei","Mo","Ge","Juan","Na","Liu","Yi","Jia","Bin","Biao","Tang","Man","Luo","Yong",
	"Chuo","Xuan","Di","Tan","Jue","Pu","Lu","Dui","Lan","Pu","Cuan","Qiang","Deng","Huo","Zhuo","Yi",
	"Cha","Biao","Zhong","Shen","Cuo","Zhi","Bi","Zi","Mo","Shu","Lv","Ji","Fu","Lang","Ke","Ren",
	"Zhen","Ji","Se","Nian","Fu","Rang","Gui","Jiao","Hao","Xi","Po","Die","Hu","Yong","Jiu","Yuan",
	"Bao","Zhen","Gu","Dong","Lu","Qu","Chi","Si","Er","Zhi","Gua","Xiu","Luan","Bo","Li","Hu",
	"Yu","Xian","Ti","Wu","Miao","An","Bei","Chun","Hu","E","Ci","Mei","Wu","Yao","Jian","Ying",
	"Zhe","Liu","Liao","Jiao","Jiu","Yu","Hu","Lu","Guan","Bing","Ding","Jie","Li","Shan","Li","You",
	"Gan","Ke","Da","Zha","Pao","Zhu","Xuan","Jia","Ya","Yi","Zhi","Lao","Wu","Cuo","Xian","Sha",
	"Zhu","Fei","Gu","Wei","Yu","Yu","Dan","La","Yi","Hou","Chai","Lou","Jia","Sao","Chi","Mo",
	"Ban","Ji","Huang","Biao","Luo","Ying","Zhai","Long","Yin","Chou","Ban","Lai","Yi","Dian","Pi","Dian",
	"Qu","Yi","Song","Xi","Qiong","Zhun","Bian","Yao","Tiao","Dou","Ke","Yu","Xun","Ju","Yu","Yi",
	"Cha","Na","Ren","Jin","Mei","Pan","Dang","Jia","Ge","Ken","Lian","Cheng","Lian","Jian","Biao","Chu",
	"Ti","Bi","Ju","Duo","Da","Bei","Bao","Lv","Bian","Lan","Chi","Zhe","Qiang","Ru","Pan","Ya",
	"Xu","Jun","Cun","Jin","Lei","Zi","Chao","Si","Huo","Lao","Tang","Ou","Lou","Jiang","Nou","Mo",
	"Die","Ding","Dan","Ling","Ning","Guo","Kui","Ao","Qin","Han","Qi","Hang","Jie","He","Ying","Ke",
	"Han","E","Zhuan","Nie","Man","Sang","Hao","Ru","Pin","Hu","Qian","Qiu","Ji","Chai","Hui","Ge",
	"Meng","Fu","Pi","Rui","Xian","Hao","Jie","Gong","Dou","Yin","Chi","Han","Gu","Ke","Li","You",
	"Ran","Zha","Qiu","Ling","Cheng","You","Qiong","Jia","Nao","Zhi","Si","Qu","Ting","Kuo","Qi","Jiao",
	"Yang","Mou","Shen","Zhe","Shao","Wu","Li","Chu","Fu","Qiang","Qing","Qi","Xi","Yu","Fei","Guo",
	"Guo","Yi","Pi","Tiao","Quan","Wan","Lang","Meng","Chun","Rong","Nan","Fu","Kui","Ke","Fu","Sou",
	"Yu","You","Lou","You","Bian","Mou","Qin","Ao","Man","Mang","Ma","Yuan","Xi","Chi","Tang","Pang",
	"Shi","Huang","Cao","Piao","Tang","Xi","Xiang","Zhong","Zhang","Shuai","Mao","Peng","Hui","Pan","Shan","Huo",
	"Meng","Chan","Lian","Mie","Li","Du","Qu","Fou","Ying","Qing","Xia","Shi","Zhu","Yu","Ji","Du",
	"Ji","Jian","Zhao","Zi","Hu","Qiong","Po","Da","Sheng","Ze","Gou","Li","Si","Tiao","Jia","Bian",
	"Chi","Kou","Bi","Xian","Yan","Quan","Zheng","Jun","Shi","Gang","Pa","Shao","Xiao","Qing","Ze","Qie",
	"Zhu","Ruo","Qian","Tuo","Bi","Dan","Kong","Wan","Xiao","Zhen","Kui","Huang","Hou","Gou","Fei","Li",
	"Bi","Chi","Su","Mie","Dou","Lu","Duan","Gui","Dian","Zan","Deng","Bo","Lai","Zhou","Yu","Yu",
	"Chong","Xi","Nie","Nv","Chuan","Shan","Yi","Bi","Zhong","Ban","Fang","Ge","Lu","Zhu","Ze","Xi",
	"Shao","Wei","Meng","Shou","Cao","Chong","Meng","Qin","Niao","Jia","Qiu","Sha","Bi","Di","Qiang","Suo",
	"Jie","Tang","Xi","Xian","Mi","Ba","Li","Tiao","Xi","Zi","Can","Lin","Zong","San","Hou","Zan",
	"Ci","Xu","Rou","Qiu","Jiang","Gen","Ji","Yi","Ling","Xi","Zhu","Fei","Jian","Pian","He","Yi",
	"Jiao","Zhi","Qi","Qi","Yao","Dao","Fu","Qu","Jiu","Ju","Lie","Zi","Zan","Nan","Zhe","Jiang",
	"Chi","Ding","Gan","Zhou","Yi","Gu","Zuo","Tuo","Xian","Ming","Zhi","Yan","Shai","Cheng","Tu","Lei",
	"Kun","Pei","Hu","Ti","Xu","Hai","Tang","Lao","Bu","Jiao","Xi","Ju","Li","Xun","Shi","Cuo",
	"Dun","Qiong","Xue","Cu","Bie","Bo","Ta","Jian","Fu","Qiang","Zhi","Fu","Shan","Li","Tuo","Jia",
	"Bo","Tai","Kui","Qiao","Bi","Xian","Xian","Ji","Jiao","Liang","Ji","Chuo","Huai","Chi","Zhi","Dian",
	"Bo","Zhi","Jian","Die","Chuai","Zhong","Ju","Duo","Cuo","Pian","Rou","Nie","Pan","Qi","Chu","Jue",
	"Pu","Fan","Cu","Zhu","Lin","Chan","Lie","Zuan","Xie","Zhi","Diao","Mo","Xiu","Mo","Pi","Hu",
	"Jue","Shang","Gu","Zi","Gong","Su","Zhi","Zi","Qing","Liang","Yu","Li","Wen","Ting","Ji","Pei",
	"Fei","Sha","Yin","Ai","Xian","Mai","Chen","Ju","Bao","Tiao","Zi","Yin","Yu","Chuo","Wo","Mian",
	"Yuan","Tuo","Zhui","Sun","Jun","Ju","Luo","Qu","Chou","Qiong","Luan","Wu","Zan","Mou","Ao","Liu",
	"Bei","Xin","You","Fang","Ba","Ping","Nian","Lu","Su","Fu","Hou","Tai","Gui","Jie","Wei","Er",
	"Ji","Jiao","Xiang","Xun","Geng","Li","Lian","Jian","Shi","Tiao","Gun","Sha","Huan","Ji","Qing","Ling",
	"Zou","Fei","Kun","Chang","Gu","Ni","Nian","Diao","Shi","Zi","Fen","Die","E","Qiu","Fu","Huang",
	"Bian","Sao","Ao","Qi","Ta","Guan","Yao","Le","Biao","Xue","Man","Min","Yong","Gui","Shan","Zun",
	"Li","Da","Yang","Da","Qiao","Man","Jian","Ju","Rou","Gou","Bei","Jie","Tou","Ku","Gu","Di",
	"Hou","Ge","Ke","Bi","Lou","Qia","Kuan","Bin","Du","Mei","Ba","Yan","Liang","Xiao","Wang","Chi",
	"Xiang","Yan","Tie","Tao","Yong","Biao","Kun","Mao","Ran","Tiao","Ji","Zi","Xiu","Quan","Jiu","Bin",
	"Huan","Lie","Me","Hui","Mi","Ji","Jun","Zhu","Mi","Qi","Ao","She","Lin","Dai","Chu","You",
	"Xia","Yi","Qu","Du","Li","Qing","Can","An","Fen","You","Wu","Yan","Xi","Qiu","Han","Zha"
};


// GB2312-80 标准规范中第一个汉字的机内码.即"啊"的机内码
const quint16 PinyinConveter::firstChCode = 0xB0A1;
// GB2312-80 标准规范中最后一个汉字的机内码.即"齄"的机内码
const quint16 PinyinConveter::lastChCode = 0xF7FE;
// GB2312-80 标准规范中最后一个一级汉字的机内码.即"座"的机内码
const quint16 PinyinConveter::lastOfOneLevelChCode = 0xD7F9;

//////////////////////////////////////////////////////////////////////////
PinyinConveter & PinyinConveter::instance()
{
	static PinyinConveter ins;
	return ins;
}


PinyinConveter::PinyinConveter()
{
}

QStringList PinyinConveter::quanpin(const QString &str)
{
	QStringList ret;
	for (int i = 0; i < str.length(); ++i)
	{
		ret << quanpin(str.at(i));
	}
	// qDebug() << Q_FUNC_INFO << str << str.length() << ret;
	return ret;
}

QStringList PinyinConveter::firstChars( const QString &str )
{
	QStringList ret;
	for (int i = 0; i < str.length(); ++i)
	{
		ret << firstChar(str.at(i));
	}

	return ret;
}

QStringList PinyinConveter::firstCharsFromQuanpin(const QStringList &strs)
{
	QStringList quanpinFirstChars;
	foreach (QString onePinyin, strs)
	{
		if (!onePinyin.isEmpty())
		{
			quanpinFirstChars.append(onePinyin.left(1));
		}
	}
	return quanpinFirstChars;
}

QString PinyinConveter::quanpin(const QChar &ch)
{
	QString ret(ch);

	do
	{
		// 标点符号、分隔符
		if (ch.isPunct() || ch.isSpace() || ch.isDigit())
			break;

		QByteArray ba = ret.toLocal8Bit();
		uint uni = ba.toHex().toUInt(0, 16);

		// 拉丁字符
		if (uni < 0xff)
			break;

		// 判断是否超过GB2312-80标准中的汉字范围
		if (uni > lastChCode || uni < firstChCode)
		{
			break;
		}
		// 如果是在一级汉字中
		else if (uni <= lastOfOneLevelChCode)
		{
			int pos = 11;
			// 将一级汉字分为12块,每块33个汉字.
			for (pos = 11 * 33; pos >= 0; pos -= 33)
			{
				// 从最后的块开始扫描,如果机内码大于块的第一个机内码,说明在此块中
				if (uni >= (quint16)oneLevelChCode[pos])
				{
					break;
				}
			}

			if (pos < 0)
			{
				break;
			}

			// 遍历块中的每个音节机内码,从最后的音节机内码开始扫描,
			// 如果音节内码小于机内码,则取此音节
			for (int i = pos + 32; i >= pos; --i)
			{

				if (uni >= (quint16)oneLevelChCode[i])
				{
					ret = oneLevelPyValue[i];
					break;
				}
			}
		}
		// 如果是在二级汉字中
		else
		{
			quint8 up = (uni >> 8) & 0xff;
			quint8 low = uni & 0xff;

			if (up < 0xD8 || up >0xF7)
				break;

			if (low <= 0xA0 || low == 0xFF)
				break;

			up -= 0xD8;
			low -= 0xA1;

			int pos = up*94+low;

			ret = twoLevelPyValue[pos];
		}
	} while(0);

	return ret;
}

QString PinyinConveter::firstChar( const QChar &ch )
{
	QString ret = quanpin(ch);
	if (!ret.isEmpty())
		ret = ret.left(1);

	return ret;
}

