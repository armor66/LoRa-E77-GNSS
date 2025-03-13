#ifndef _FONTS_H
#define _FONTS_H

//---- для экономии памяти шрифты которые не используються закоментировать -------------------------------------
			// Шрифт имеет кирилицу и латиницу
			#define FONT_6x8		//  6 x 8 pixels font size structure
			#define FONT_7x9		//  7 x 9 pixels font size structure
			#define FONT_11x18		//  11 x 18 pixels font size structure
//			#define FONT_16x26		//  16 x 26 pixels font size structure
//			#define FONT_16x28		//  16 x 28 pixels font size structure only numbers
//--------------------------------------------------------------------------------------------------------------

#include "main.h"
#include "string.h"
#include "stdlib.h"

typedef struct {
	uint8_t FontWidth;    /*!< Font width in pixels */
	uint8_t FontHeight;   /*!< Font height in pixels */
	uint8_t CharsMax;
	uint16_t FontPexels;
	const uint16_t *data; /*!< Pointer to data font data array */
} FontDef_t;

typedef struct {
	uint16_t Length;      /*!< String length in units of pixels */
	uint16_t Height;      /*!< String height in units of pixels */
} FONTS_SIZE_t;

#ifdef	FONT_6x8
extern FontDef_t Font_6x8;
#endif

#ifdef	FONT_7x9
extern FontDef_t Font_7x9;
#endif

 #ifdef	FONT_11x18
extern FontDef_t Font_11x18;
#endif

 #ifdef	FONT_16x26
extern FontDef_t Font_16x26;
#endif

 #ifdef	FONT_16x28
// Только цифры -- only numbers
extern FontDef_t Font_16x28;
#endif

/**
 * @brief  Calculates string length and height in units of pixels depending on string and font used
 * @param  *str: String to be checked for length and height
 * @param  *SizeStruct: Pointer to empty @ref FONTS_SIZE_t structure where informations will be saved
 * @param  *Font: Pointer to @ref FontDef_t font used for calculations
 * @retval Pointer to string used for length and height
 */
char* FONTS_GetStringSize(char* str, FONTS_SIZE_t* SizeStruct, FontDef_t* Font);

#endif	/*	_FONTS_H */
