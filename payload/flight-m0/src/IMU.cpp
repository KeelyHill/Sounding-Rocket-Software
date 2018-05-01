
#ifndef IMU_CPP
#define IMU_CPP

#include <../lib/Adafruit_LSM9DS1/Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <Madgwick.h>
#include <Mahony.h>

#include "global.h"


class IMU {

	Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();  // i2c sensor
	// Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(SS_ACCEL, SS_ACCEL);
	// Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1(LSM9DS1_XGCS, LSM9DS1_MCS);
	sensors_event_t accel, mag, gyro, temp;
	uint32_t lastIMUSamp = 0; // millis

	/* Calibration values */

	// Offsets applied to raw x/y/z mag values
	const float magOffsets[3]            = { 66.2F, 4.45, -5.99F };

	// Soft iron error compensation matrix
	const float magSoftironMatrix[3][3] = { {  1.028,  0.056,  0.202 },
	                                    {  0.056,  0.910, 0.038 },
	                                    {  0.202, 0.038,  1.1113 } };

	const float magFieldStrength        = 52.85F;

	// Offsets applied to compensate for gyro zero-drift error for x/y/z
	const float gyroZeroOffsets[3]      = { 0.0F, 0.0F, 0.0F };


public:
	Mahony filter; // lighter weight
	// Madgwick filter;

	/** Starts the LSM device, if successful also sets sensor ranges. */
	bool begin() {
		if (lsm.begin()) {

			// accelerometer range
			lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G); // 2, 4, 8, 16

			// magnetometer sensitivity
			lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS); // 4, 8, 12, 16

			// Setup the gyroscope
			lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS); // 245, 500, 2000

			return true;
		}
		return false;
	}

	/** Samples the LSM device (over whichever bus) and updates the orientation filter. */
	void sample() {
		lsm.getEvent(&accel, &mag, &gyro, &temp);
		double delta_IMU_time = millis()/1000.0f - lastIMUSamp/1000.0f;
		lastIMUSamp = millis();
		// float heading = atan2(mag.magnetic.y, mag.magnetic.x); // atan2(y/x)
		// Serial.print(heading * 180/M_PI); Serial.print(" - "); Serial.print(mag.magnetic.x); Serial.print(" "); Serial.print(mag.magnetic.y); Serial.print(" "); Serial.println(mag.magnetic.z);

		// Apply mag offset compensation (base values in uTesla)
		float mx_ = mag.magnetic.x - magOffsets[0];
		float my_ = mag.magnetic.y - magOffsets[1];
		float mz_ = mag.magnetic.z - magOffsets[2];

		// Apply mag soft iron error compensation
		float mx = mx_ * magSoftironMatrix[0][0] + my_ * magSoftironMatrix[0][1] + mz_ * magSoftironMatrix[0][2];
		float my = mx_ * magSoftironMatrix[1][0] + my_ * magSoftironMatrix[1][1] + mz_ * magSoftironMatrix[1][2];
		float mz = mx_ * magSoftironMatrix[2][0] + my_ * magSoftironMatrix[2][1] + mz_ * magSoftironMatrix[2][2];

		// Apply gyro zero-rate error compensation
		float gx = gyro.gyro.x + gyroZeroOffsets[0];
		float gy = gyro.gyro.y + gyroZeroOffsets[1];
		float gz = gyro.gyro.z + gyroZeroOffsets[2];

		// The filter library expects gyro data in degrees/s, but adafruit sensor
		// uses rad/s so we need to convert them first (or adapt the filter lib
		// where they are being converted)
		gx *= 57.2958F;
		gy *= 57.2958F;
		gz *= 57.2958F;

		filter.update(gx, gy, gz,
	                accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
	                mx, my, mz,
					delta_IMU_time);

		//
		// filter.updateIMU(gx, gy, gz,
	    //             accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
		// 			delta_IMU_time);


		if (DEBUG) {
			// Serial.print(" dT (ms):");
			// Serial.println(delta_IMU_time);
		}

	}

	void debugPrint() {
		if (DEBUG) {
			float roll = filter.getRoll();
			float pitch = filter.getPitch();
			float yaw = filter.getYaw();

			Serial.print(millis());
			Serial.print(" - Orientation: ");
			Serial.print(yaw);
			Serial.print(" ");
			Serial.print(pitch);
			Serial.print(" ");
			Serial.println(roll);
		}
	}

	/** Prints raw data to serial for calibration (call after sample()).
	https://learn.adafruit.com/nxp-precision-9dof-breakout/calibration-usb
	*/
	void calibrationPrint() {
		Serial.print("Raw:");
		Serial.print((int)(accel.acceleration.x * 1000));
		Serial.print(",");
		Serial.print((int)(accel.acceleration.y * 1000));
		Serial.print(",");
		Serial.print((int)(accel.acceleration.z * 1000));
		Serial.print(",");

		Serial.print((int)(gyro.gyro.x * 1000));
		Serial.print(",");
		Serial.print((int)(gyro.gyro.y * 1000));
		Serial.print(",");
		Serial.print((int)(gyro.gyro.z * 1000));
		Serial.print(",");

		Serial.print((int)(mag.magnetic.x * 1000));
		Serial.print(",");
		Serial.print((int)(mag.magnetic.y * 1000));
		Serial.print(",");
		Serial.print((int)(mag.magnetic.z * 1000));
		Serial.println();
	}
};


#endif
