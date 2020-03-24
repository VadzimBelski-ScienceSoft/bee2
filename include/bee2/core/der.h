/*
*******************************************************************************
\file der.h
\brief Distinguished Encoding Rules
\project bee2 [cryptographic library]
\author (C) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2014.04.21
\version 2019.06.13
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file der.h
\brief Отличительные правила кодирования
*******************************************************************************
*/

#ifndef __BEE2_DER_H
#define __BEE2_DER_H

#include "bee2/defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\file der.h DER-кодирование

Поддержано кодирование по правилам АСН.1. Согласно этим правилам, структуры 
данных представляются в формате TLV: сначала идет октеты тега (T), затем 
октеты длины (L), а после этого L октетов значения (V).

Тег задается, как минимум, одним октетом, в котором младшие 5 битов
представляют номер, 6-й бит является признаком примитивности /
конструктивности (если бит установлен, то V не содержит / содержит
вложенные структуры TLV), а старшие два бита определяют класс тега.

Если не все 5 младших битов первого октета тега равняются 1,
то первый октет является единственным (короткий тег). Короткие теги
могут иметь номера от 0 до 30.

Если все 5 младших битов типа первого октета тега равняются 1
(длинный тег), то номер тега представляется в виде
	\sum {i = 0}^{r - 1}t_i 128^i, 0 <= t_i < 128, t_{r - 1} != 0,
и кодируется октетами
	(t_{r - 1} | 128), ..., (t_1 | 128), t_0,
следующими сразу за первым октетом тега.

Длинные теги должен применяться только для номеров >= 31. В частности,
если r == 1, то t_0 >= 31.

Ограничение реализации: тег задается словом u32. Номер тега задается
младшими 5 битами слова, если не все эти биты единичные (короткий тег),
либо старшими 24 битами, в противном случае (длинный тег). Если в коротком
теге некоторый из старших 24 битов ненулевой, то это считается ошибкой
формата. Если в длинном теге старшие 24 бита соответствуют числу < 31, то
это также считается ошибкой формата.

Если L < 128, то длина может кодироваться одним октетом, содержащим L
(короткая явная форма). Если L >= 128, то длина представляется в виде
	L = \sum {i = 0}^{r - 1}l_i 256^i, 0 <= l_i < 256,
и кодируется r + 1 октетами
	(r | 128), l_{r - 1},..., l_0
(длинная явная форма). Заранее неизвестная длина кодируется октетом 128
(неявная форма). Значение 255 первого октета длины зарезервировано и не
должно использоваться.

При кодировании примитивных типов (см. 6-й бит тега) должна использоваться
явная форма длины. Других ограничений в базовых правилах кодирования АСН.1,
которые называются BER (Basic Encoding Rules), нет. Например, допускается
выбор длинной формы для L < 128, допускается использование в длинной
форме нулевых старших октетов (l_{r - 1} может равняться 0).

Реализованные в настоящем модуле правила DER (Distinguished Encoding Rules) 
снимают неоднозначности. По правилам DER длина должна кодироваться всегда 
в явной форме и всегда с помощью минимального числа октетов. Последнее 
означает, что при L < 128 должна применяться короткая форма, 
а при L >= 128 -- длинная форма с l_{r - 1} != 0.

Ограничение реализации: длина укладывается в size_t.
*******************************************************************************
*/

/*!	\brief Кодирование

	Определяется число октетов в DER-коде значения [len]value с тегом tag. 
	Если der != 0, то DER-код размещается по этому адресу.
	\pre Если der != 0, то value != 0 и по адресу der зарезервировано
	derEncode(0, tag, value, len) октетов.
	\return Число октетов в DER-коде или SIZE_MAX в случае ошибки.
	\remark Ошибкой является неверный формат tag.
	\remark Разрешаются вызовы derEncode(0, tag, value, len),
	derEncode(0, tag, 0, len).
*/
size_t derEncode(
	octet der[],		/*!< [out] DER-код */
	u32 tag,			/*!< [in] тег */
	const void* value,	/*!< [in] значение */
	size_t len			/*!< [in] длина value в октетах */
);

/*!	\brief Корректный код?

	Проверяется корректность DER-кода [count]der. Проверяются следующие 
	условия:
	-	тег T и длина L закодированы по правилам ACH.1;
	-	тег укладывается в u32 (ограничение реализации);
	-	длина укладывается в size_t (органичение реализации);
	-	count действительно является длиной кода.
	.
	\return Признак корректности.
	\remark Содержимое V не проверяется.
*/
bool_t derIsValid(
	const octet der[],	/*!< [in] DER-код */
	size_t count		/*!< [in] длина der в октетах */
);

/*!	\brief Корректный код с заданным тегом?

	Проверяется корректность DER-кода [count]der с заданным тегом tag. 
	Проверяются следующие условия:
	-	derIsValid(der, count) == TRUE;
	-	der имеет тег tag.
	.
	\return Признак корректности.
	\remark Содержимое V не проверяется.
*/
bool_t derIsValid2(
	const octet der[],	/*!< [in] DER-код */
	size_t count,		/*!< [in] длина der в октетах */
	u32 tag				/*!< [in] тег */
);

/*!	\brief Длина кода

	Определяется точная длина DER-кода [count]der. Точная длина является 
	суммой длин полей T, L и V (длина V задана в L). При определении 
	длины проводится проверка корректности полей T и L. 
	\return Длина DER-кода или SIZE_MAX в случае ошибки формата.
*/
size_t derSize(
	const octet der[],	/*!< [in] DER-код */
	size_t count		/*!< [in] оценка длины сверху */
);

/*!	\brief Декодирование

	Определяется длина значения, представленного DER-кодом [count]der. 
	Если tag != 0 и (или) value != 0, то тег и (или) значение размещаются 
	по этим адресам.
	\pre Если tag != 0, то буферы tag и der не пересекаются.
	\pre Если tag != 0 и value != 0, то буферы tag и value не пересекаются.
	\pre Если value != 0, то по адресу value зарезервировано
	derDecode(0, 0, der, count) октетов.
	\expect{SIZE_MAX} derIsValid(der, count).
	\return Число октетов памяти или SIZE_MAX в случае ошибки формата.
*/
size_t derDecode(
	u32* tag,			/*!< [out] тег */
	void* value,		/*!< [out] значение */
	const octet der[],	/*!< [in] DER-код */
	size_t count		/*!< [in] длина der в октетах */
);

/*!	\brief Разбор кода

	Определяется длина значения, представленного DER-кодом [count]der. 
	Если tag != 0 и (или) value != 0, то тег и (или) указатель на значение 
	размещаются по этим адресам (value указывает на буфер памяти внутри 
	буфера der).
	\pre Если tag != 0, то буферы tag и der не пересекаются.
	\expect{SIZE_MAX} derIsValid(der, count).
	\return Число октетов памяти или SIZE_MAX в случае ошибки формата.
*/
size_t derDecode2(
	u32* tag,				/*!< [out] тег */
	const octet** value,	/*!< [out] указатель на значение */
	const octet der[],		/*!< [in] DER-код */
	size_t count			/*!< [in] длина der в октетах */
);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BEE2_DER_H */
