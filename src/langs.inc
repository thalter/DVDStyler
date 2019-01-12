// included in src/Languages.cpp, encoding=UTF8
#include "../locale/bitmaps/arab_league.png.h"
#include "../locale/bitmaps/argentina.png.h"
#include "../locale/bitmaps/armenia.png.h"
#include "../locale/bitmaps/bulgaria.png.h"
#include "../locale/bitmaps/catalonia.png.h"
#include "../locale/bitmaps/china.png.h"
#include "../locale/bitmaps/taiwan.png.h"
#include "../locale/bitmaps/czech.png.h"
#include "../locale/bitmaps/denmark.png.h"
#include "../locale/bitmaps/germany.png.h"
#include "../locale/bitmaps/united_kingdom.png.h"
#include "../locale/bitmaps/spain.png.h"
#include "../locale/bitmaps/basque_country.png.h"
#include "../locale/bitmaps/finland.png.h"
#include "../locale/bitmaps/france.png.h"
#include "../locale/bitmaps/greece.png.h"
#include "../locale/bitmaps/italy.png.h"
#include "../locale/bitmaps/japan.png.h"
#include "../locale/bitmaps/south_korea.png.h"
#include "../locale/bitmaps/hungary.png.h"
#include "../locale/bitmaps/macedonia.png.h"
#include "../locale/bitmaps/netherlands.png.h"
#include "../locale/bitmaps/norway.png.h"
#include "../locale/bitmaps/poland.png.h"
#include "../locale/bitmaps/portugal.png.h"
#include "../locale/bitmaps/brazil.png.h"
#include "../locale/bitmaps/romania.png.h"
#include "../locale/bitmaps/russia.png.h"
#include "../locale/bitmaps/slovakia.png.h"
#include "../locale/bitmaps/slovenia.png.h"
#include "../locale/bitmaps/serbia.png.h"
#include "../locale/bitmaps/sri_lanka.png.h"
#include "../locale/bitmaps/sweden.png.h"
#include "../locale/bitmaps/turkey.png.h"
#include "../locale/bitmaps/ukraine.png.h"
#include "../locale/bitmaps/uzbekistan.png.h"
#include "../locale/bitmaps/vietnam.png.h"

struct Lang {
	const char* name;
	wxLanguage id;
	const unsigned char* bitmap;
	unsigned int bmpLength;
};

Lang s_langs[] = {
	{ "‏اللغة العربية‎", wxLANGUAGE_ARABIC, (const unsigned char *) arab_league_png, sizeof(arab_league_png)},
	{ "Հայոց լեզու", wxLANGUAGE_ARMENIAN, (const unsigned char *) armenia_png, sizeof(armenia_png)},
	{ "Български", wxLANGUAGE_BULGARIAN, (const unsigned char *) bulgaria_png, sizeof(bulgaria_png)},
	{ "Català", wxLANGUAGE_CATALAN, (const unsigned char *) catalonia_png, sizeof(catalonia_png)},
	{ "简体中文", wxLANGUAGE_CHINESE_SIMPLIFIED, (const unsigned char *) china_png, sizeof(china_png)},
	{ "繁體中文", wxLANGUAGE_CHINESE_TRADITIONAL, (const unsigned char *) taiwan_png, sizeof(taiwan_png)},
	{ "Česky", wxLANGUAGE_CZECH, (const unsigned char *) czech_png, sizeof(czech_png)},
	{ "Dansk", wxLANGUAGE_DANISH, (const unsigned char *) denmark_png, sizeof(denmark_png)},
	{ "Deutsch", wxLANGUAGE_GERMAN, (const unsigned char *) germany_png, sizeof(germany_png)},
	{ "English", wxLANGUAGE_ENGLISH, (const unsigned char *) united_kingdom_png, sizeof(united_kingdom_png)},
	{ "Español", wxLANGUAGE_SPANISH, (const unsigned char *) spain_png, sizeof(spain_png)},
	{ "Español (Argentina)", wxLANGUAGE_SPANISH_ARGENTINA, (const unsigned char *) argentina_png, sizeof(argentina_png)},
	{ "Euskara", wxLANGUAGE_BASQUE, (const unsigned char *) basque_country_png, sizeof(basque_country_png)},
	{ "Français", wxLANGUAGE_FRENCH, (const unsigned char *) france_png, sizeof(france_png)},
	{ "Ελληνικά", wxLANGUAGE_GREEK, (const unsigned char *) greece_png, sizeof(greece_png)},
	{ "Italiano", wxLANGUAGE_ITALIAN, (const unsigned char *) italy_png, sizeof(italy_png)},
	{ "日本語 ", wxLANGUAGE_JAPANESE, (const unsigned char *) japan_png, sizeof(japan_png)},
	{ "한국어", wxLANGUAGE_KOREAN, (const unsigned char *) south_korea_png, sizeof(south_korea_png)},
	{ "Magyar", wxLANGUAGE_HUNGARIAN, (const unsigned char *) hungary_png, sizeof(hungary_png)},
	{ "Македонски", wxLANGUAGE_MACEDONIAN, (const unsigned char *) macedonia_png, sizeof(macedonia_png)},
	{ "Nederlands", wxLANGUAGE_DUTCH, (const unsigned char *) netherlands_png, sizeof(netherlands_png)},
	{ "Norsk", wxLANGUAGE_NORWEGIAN_BOKMAL, (const unsigned char *) norway_png, sizeof(norway_png)},
	{ "Polski", wxLANGUAGE_POLISH, (const unsigned char *) poland_png, sizeof(poland_png)},
	{ "Português", wxLANGUAGE_PORTUGUESE, (const unsigned char *) portugal_png, sizeof(portugal_png)},
	{ "Português (Brasil)", wxLANGUAGE_PORTUGUESE_BRAZILIAN, (const unsigned char *) brazil_png, sizeof(brazil_png)},
	{ "Română", wxLANGUAGE_ROMANIAN, (const unsigned char *) romania_png, sizeof(romania_png)},
	{ "Pyccкий", wxLANGUAGE_RUSSIAN, (const unsigned char *) russia_png, sizeof(russia_png)},
	{ "සිංහල", wxLANGUAGE_SINHALESE, (const unsigned char *) sri_lanka_png, sizeof(sri_lanka_png)},
	{ "Slovenčina", wxLANGUAGE_SLOVAK, (const unsigned char *) slovakia_png, sizeof(slovakia_png)},
	{ "Slovenščina", wxLANGUAGE_SLOVENIAN, (const unsigned char *) slovenia_png, sizeof(slovenia_png)},
	{ "Srpski", wxLANGUAGE_SERBIAN, (const unsigned char *) serbia_png, sizeof(serbia_png)},
	{ "Suomi", wxLANGUAGE_FINNISH, (const unsigned char *) finland_png, sizeof(finland_png)},
	{ "Svenska", wxLANGUAGE_SWEDISH, (const unsigned char *) sweden_png, sizeof(sweden_png)},
	{ "Türkçe", wxLANGUAGE_TURKISH, (const unsigned char *) turkey_png, sizeof(turkey_png)},
	{ "Украї́нська", wxLANGUAGE_UKRAINIAN, (const unsigned char *) ukraine_png, sizeof(ukraine_png)},
	{ "Oʻzbek", wxLANGUAGE_UZBEK, (const unsigned char *) uzbekistan_png, sizeof(uzbekistan_png)},
	{ "tiếng Việt", wxLANGUAGE_VIETNAMESE, (const unsigned char *) vietnam_png, sizeof(vietnam_png)}
};
