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

 * 	QMC5883L.c
 *  Created on: Jan 31, 2022
 */

#include "QMC5883L.h"

static const float QMC5883L_LSB_2G = 12000.0f;
static const float QMC5883L_LSB_8G = 3000.0f;

/*static inline CONCAT_BYTES(uint8_t msb, uint8_t lsb) {
	return (((uint16_t) msb << 8) | (uint16_t) lsb);
}*/

uint8_t QMC5883L_Init(I2C_Connection *_i2c, QMC5883L_t *dev) {
	if (_i2c->status == PORT_FREE) { //send setup
		if (_i2c->buffer.lockState != FREE) {//send setup
			return 0;
		}
		switch (dev->step) {
			case 0: //set reset period don't give a fuck
				_i2c->buffer.lockState = BLOCKED;
				dev->status = INIT;
				_i2c->addr = dev->addr;
				_i2c->mode = I2C_MODE_WRITE;
				_i2c->reg = QMC5883L_REG_PERIOD;
				_i2c->len = 1;
				PutOne(&_i2c->buffer, 0x01); //recommended magic number RTFM
				dev->step = 1;
				break;
			case 1: //setup sensor gain and sample rate interrupt
				_i2c->addr = dev->addr;
				_i2c->mode = I2C_MODE_WRITE;
				_i2c->reg = QMC5883L_REG_CFG_A;
				_i2c->len = 2;
				PutOne(&_i2c->buffer,
						QMC5883L_MODE_CONTINUOUS | QMC5883L_ODR_50HZ
								| QMC5883L_RNG_8G | QMC5883L_SAMPLES_512);
				PutOne(&_i2c->buffer, 0x00);
				dev->step = 2;
				break;
			case 2:
				dev->status = OK;
				dev->step = 0;
				_i2c->buffer.lockState = FREE;
				return 1;
				break;
			default:
				dev->step = 0;
				return 0;
		}
		I2C_Start_IRQ(_i2c);
	}
	return 0;
}

uint8_t QMC5883L_GetData(I2C_Connection *_i2c, QMC5883L_t *dev) {
	uint8_t dt[QMC5883L_DATA_LEN];
	if (_i2c->status == PORT_FREE) {
		_i2c->addr = dev->addr;
		switch (dev->step) {
			case 0: //read data
				_i2c->reg = QMC5883L_REG_OUT_X_L;
				_i2c->len = QMC5883L_DATA_LEN;
				_i2c->mode = I2C_MODE_READ;
				dev->step = 1;
				break;
			case 1: //calculate data
				GetMulti(&_i2c->buffer, dt, QMC5883L_DATA_LEN);
				dev->raw.X = (int16_t)(((uint16_t)dt[1])<< 8 | (uint16_t)dt[0]);// CONCAT_BYTES(dt[1], dt[0]);
				dev->raw.Y = (int16_t)(((uint16_t)dt[3])<< 8 | (uint16_t)dt[2]);// CONCAT_BYTES(dt[3], dt[2]);
				dev->raw.Z = (int16_t)(((uint16_t)dt[5])<< 8 | (uint16_t)dt[4]);// CONCAT_BYTES(dt[5], dt[4]);
				dev->data.X = (float) dev->raw.X / QMC5883L_LSB_8G;
				dev->data.Y = (float) dev->raw.Y / QMC5883L_LSB_8G;
				dev->data.Z = (float) dev->raw.Z / QMC5883L_LSB_8G;
				dev->status = OK;
				dev->step = 0;
				return 1;
				break;
			default:
				dev->step = 0;
				return 0;
		}
		I2C_Start_IRQ(_i2c);
	}
	return 0;
}
