/*********************************************************************************
   Original author: Alexandr Pochtovy<alex.mail.prime@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 * 	QMC5883L.h
 *  Created on: Jan 31, 2022
*/

#ifndef QMC5883L_H_
#define QMC5883L_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "I2C_Master/MyI2C.h"
#include "QMC5883L_Register.h"
/* состояние процесса обмена данными с устройством как с отдельным элементом сети
 * 	применяется для отображения состояния процесса работы с устройством для главного кода
 */
typedef enum QMC5883L_status_t {//состояние устройства
	QMC_Init,		//устройство не настроено
	QMC_OK,		//устройство готово к опросу
	QMC_Faulth	//устройство неисправно
} QMC5883L_status;

/*	состояние обмена данными с устройством, использовать для завершения функции работы с устройством */
typedef enum QMC5883L_Connect_Status_t {
	QMC_Processing, //выполняется работа с устройством: обмен данными, обработка результатов
	QMC_Complite	//работа с устройством завершена, данные считаны/записаны корректно
} QMC5883L_Connect_Status;

typedef struct QMC5883L_raw_data_t {
	int16_t X;
	int16_t Y;
	int16_t Z;
	uint8_t status;
} QMC5883L_raw_data;

typedef struct QMC5883L_data_t {
	float X;
	float Y;
	float Z;
} QMC5883L_data;
//common data struct for sensor
typedef struct QMC5883L_dev_t {
	const uint8_t addr;
	uint8_t step;
	QMC5883L_status status;
	QMC5883L_raw_data raw;
	QMC5883L_data data;
} QMC5883L_dev;

//INITIALIZATION	================================================================
QMC5883L_Connect_Status QMC5883L_Init(I2C_Connection *_i2c, QMC5883L_dev *dev, uint8_t *pbuffer);
QMC5883L_Connect_Status QMC5883L_GetData(I2C_Connection *_i2c, QMC5883L_dev *dev, uint8_t *pbuffer);

#ifdef __cplusplus
}
#endif
#endif /* QMC5883L_H_ */
