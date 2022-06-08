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

static const uint8_t QMC5883L_DEFAULT_ADDRESS = 0x1A;
static const uint16_t QMC5883L_LSB_2G = 12000;
static const uint16_t QMC5883L_LSB_8G = 3000;

QMC5883L_Connect_Status QMC5883L_Init(I2C_Connection *_i2c, QMC5883L_dev *dev, uint8_t *pbuffer) {
	if (_i2c->i2cStatus == I2C_Bus_Free) {//send setup
		dev->status = QMC_Init;
		_i2c->addr = dev->addr;
		_i2c->rxtxp = pbuffer;
		_i2c->mode = I2C_MODE_WRITE;
		switch (dev->step) {
		case 0://set reset period don't give a fuck
			_i2c->reg = QMC5883L_REG_PERIOD;
			_i2c->len = 1;
			_i2c->rxtxp[0] = 0x01;//reccomended magic number RTFM
			dev->step = 1;
			break;
		case 1://setup sensor gain and sample rate
			_i2c->reg = QMC5883L_REG_CFG_A;
			_i2c->len = 1;
			_i2c->rxtxp[0] = 	QMC5883L_MODE_CONTINUOUS | 
												QMC5883L_ODR_50HZ | 
												QMC5883L_RNG_8G |
												QMC5883L_SAMPLES_512;
			dev->step = 2;
			break;
		case 2:
			dev->status = QMC_OK;
			dev->step = 0;
			return QMC_Complite;
			break;
		default:
			dev->step = 0;
			break;
		}
    I2C_Start_IRQ(_i2c);
	}
	return QMC_Processing;
}

QMC5883L_Connect_Status QMC5883L_GetData(I2C_Connection *_i2c, QMC5883L_dev *dev, uint8_t *pbuffer) {
	if (_i2c->i2cStatus == I2C_Bus_Free) {
		_i2c->rxtxp = pbuffer;
    switch (dev->step) {
		case 0://read data
			_i2c->addr = dev->addr;
			_i2c->reg = QMC5883L_REG_OUT_X_L;
			_i2c->len = 6;
			_i2c->mode = I2C_MODE_READ;
			dev->step = 1;
			break;

		case 1://calculate data
			dev->raw.X = ((int16_t)_i2c->rxtxp[1]<<8) | _i2c->rxtxp[0];
			dev->raw.Z = ((int16_t)_i2c->rxtxp[3]<<8) | _i2c->rxtxp[2];
			dev->raw.Y = ((int16_t)_i2c->rxtxp[5]<<8) | _i2c->rxtxp[4];
			dev->data.X = (float)dev->raw.X / QMC5883L_LSB_8G;
			dev->data.Y = (float)dev->raw.Y / QMC5883L_LSB_8G;
			dev->data.Z = (float)dev->raw.Z / QMC5883L_LSB_8G;
			dev->status = QMC_OK;
    	 dev->step = 0;
			return QMC_Complite;
			break;
		default:
			dev->step = 0;
            break;
		}
        I2C_Start_IRQ(_i2c);
	}
	return QMC_Processing;
}
