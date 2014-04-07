// trigger_loops.cpp

#include "dtb_hal.h"
#include "pixel_dtb.h"
#include "dtb_config.h"
#include "rpc.h"

#define TRIGGER_DELAY 4

// -------- Helper Functions -------------------------------

char ROC_TRIM_BITS[MOD_NUMROCS*ROC_NUMCOLS*ROC_NUMROWS];
char ROC_I2C_ADDRESSES[MOD_NUMROCS];

// Return the Row to be pulsed with a calibrate signal
// If "xtalk" is set, move the row by one count up or down
uint8_t CTestboard::GetXtalkRow(uint8_t row, bool xtalk) {
  
  if (xtalk) {
    if (row == ROC_NUMROWS - 1) { return (row - 1); }
    else { return (row + 1); }
  }
  else { return row; }
}

// Return the calibrated DAC value - the 8bit DAC range is implemented as
// set of transistors. This results in some jumps in the spectrum of a DAC
// which needs to be flattened by flipping some DAC values. This is done
// by this function, 4bit DACs are also passed, but there the range is flat.
size_t CTestboard::CalibratedDAC(size_t value) {
  static const int dac[256] = {0, 1, 2, 3, 4, 5, 6, 8, 7, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 23, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 40, 39, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 56, 55, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 72, 71, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 88, 87, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 104, 103, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 120, 119, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 136, 135, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 152, 151, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 184, 183, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 200, 199, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 216, 215, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 232, 231, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 248, 247, 249, 250, 251, 252, 253, 254, 255};
  return dac[value];
}

// Setup of data storage structures in the NIOS stack. Stores all
// ROC I2C addresses to be accessed later by functions which retrieve
// trim values for a specific ROC:
bool CTestboard::SetI2CAddresses(vector<uint8_t> &roc_i2c) {

	if(roc_i2c.size() > MOD_NUMROCS) { return false; }
	for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
		ROC_I2C_ADDRESSES[roc] = roc_i2c.at(roc);
	}
	return true;
}

// Upload all trimvalues of one ROC to the NIOS core to store them for looping
// over multiple pixels:
bool CTestboard::SetTrimValues(uint8_t roc_i2c, vector<int8_t> &trimvalues) {

	// Get the array index of the requested ROC via its I2C address:
	size_t index = 0;
	for(index = 0; index < MOD_NUMROCS; index++) {
		if(ROC_I2C_ADDRESSES[index] == roc_i2c) { break; }
	}

	// Write the trim values into the corresponding array section:
	for(size_t val = 0; val < trimvalues.size(); val++) {
		ROC_TRIM_BITS[index*ROC_NUMROWS*ROC_NUMCOLS + val] = trimvalues.at(val);
	}
	return true;
}

// Read back the trim value of one specific pixel on a given ROC:
uint8_t CTestboard::GetTrimValue(uint8_t roc_i2c, uint8_t column, uint8_t row) {

	// Lookup the index of this particular ROC:
	size_t index = 0;
	for(index = 0; index < MOD_NUMROCS; index++) {
		if(ROC_I2C_ADDRESSES[index] == roc_i2c) { break; }
	}

	// Lookup the trim bits value of this particular pixel:
	int8_t value = ROC_TRIM_BITS[index*ROC_NUMROWS*ROC_NUMCOLS + column*ROC_NUMROWS + row];

	if(value < 0) return 0xf;
	else return value;
}

bool CTestboard::GetMaskState(uint8_t roc_i2c, uint8_t column, uint8_t row) {

	// Lookup the mask state of this particular pixel from a huge map stored somewhere in the NIOS
	// mask[MOD_NUMROCS*ROC_NUMCOLS*ROC_NUMCOLS]
	return false;
}


// -------- Simple Calibrate Functions for Maps -------------------------------

void CTestboard::LoopMultiRocAllPixelsCalibrate(vector<uint8_t> &roc_i2c, uint16_t nTriggers, uint16_t flags) {

  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) {
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Chip_Mask();
    }
  }

  // Loop over all columns:
  for (uint8_t col = 0; col < ROC_NUMCOLS; col++) {

    // Enable this column on every configured ROC:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Col_Enable(col, true);
    }

    // Loop over all rows:
    for (uint8_t row = 0; row < ROC_NUMROWS; row++) {

      // Set the calibrate bits on every configured ROC
      // Take into account both Xtalks and Cals flags
      for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
        roc_I2cAddr(roc_i2c.at(roc));
	// If masked, enable the pixel:
	if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(col, row, 15);
	roc_Pix_Cal(col, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));
      }

      // Send the triggers:
      uDelay(5);
      for (uint16_t trig = 0; trig < nTriggers; trig++) {
	Pg_Single();
	// Delay the next trigger, depending in the data traffic we expect:
	uDelay(TRIGGER_DELAY*roc_i2c.size());
      }

      // Clear the calibrate signal on every ROC configured
      for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	roc_I2cAddr(roc_i2c.at(roc));
	if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(col, row);
	roc_ClrCal();
      }
    } // Loop over all rows

    // Disable this column on every ROC configured:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Col_Enable(col, false);
    }

  } // Loop over all columns
}

void CTestboard::LoopMultiRocOnePixelCalibrate(vector<uint8_t> &roc_i2c, uint8_t column, uint8_t row, uint16_t nTriggers, uint16_t flags) {

  // Enable this column on every configured ROC:
  // Set the calibrate bits on every configured ROC
  // Take into account both Xtalks and Cals flags
  for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
    roc_I2cAddr(roc_i2c.at(roc));
    if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();
    roc_Col_Enable(column, true);
    // If masked, enable the pixel:
    if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(column, row, 15);
    roc_Pix_Cal(column, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));
  }

  // Send the triggers:
  uDelay(5);
  for (uint16_t trig = 0; trig < nTriggers; trig++) {
    Pg_Single();
    // Delay the next trigger, depending in the data traffic we expect:
    uDelay(TRIGGER_DELAY*roc_i2c.size());
  }

  // Clear the calibrate signal on every ROC configured
  // Disable this column on every ROC configured:
  for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
    roc_I2cAddr(roc_i2c.at(roc));
    if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(column, row);
    roc_ClrCal();
    roc_Col_Enable(column, false);
  }
}

void CTestboard::LoopSingleRocAllPixelsCalibrate(uint8_t roc_i2c, uint16_t nTriggers, uint16_t flags) {

  // Set the I2C output to the correct ROC:
  roc_I2cAddr(roc_i2c);
  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();

  // Loop over all columns:
  for (uint8_t col = 0; col < ROC_NUMCOLS; col++) {

    // Enable this column on the configured ROC:
    roc_Col_Enable(col, true);

    // Loop over all rows:
    for (uint8_t row = 0; row < ROC_NUMROWS; row++) {

      // If masked, enable the pixel:
      if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(col, row, 15);

      // Set the calibrate bits
      // Take into account both Xtalks and Cals flags
      roc_Pix_Cal(col, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));

      // Send the triggers:
      uDelay(5);
      for (uint16_t trig = 0; trig < nTriggers; trig++) {
	Pg_Single();
	uDelay(TRIGGER_DELAY);
      }

      // Clear the calibrate signal
      if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(col, row);
      roc_ClrCal();
    } // Loop over all rows

    // Disable this column on every ROC configured:
    roc_Col_Enable(col, false);

  } // Loop over all columns
}

void CTestboard::LoopSingleRocOnePixelCalibrate(uint8_t roc_i2c, uint8_t column, uint8_t row, uint16_t nTriggers, uint16_t flags) {

  // Enable this column on the configured ROC:
  // Set the calibrate bits on every configured ROC
  // Take into account both Xtalks and Cals flags
  roc_I2cAddr(roc_i2c);
  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();

  roc_Col_Enable(column, true);
  // If masked, enable the pixel:
  if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(column, row, 15);
  roc_Pix_Cal(column, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));

  // Send the triggers:
  uDelay(5);
  for (uint16_t trig = 0; trig < nTriggers; trig++) {
    Pg_Single();
    uDelay(TRIGGER_DELAY);
  }

  // Clear the calibrate signal on thr ROC configured
  // Disable this column on the ROC configured:
  if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(column, row);
  roc_ClrCal();
  roc_Col_Enable(column, false);
}


// -------- Trigger Loop Functions for 1D Dac Scans -------------------------------

void CTestboard::LoopMultiRocAllPixelsDacScan(vector<uint8_t> &roc_i2c, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high) {

  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) {
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Chip_Mask();
    }
  }

  // Loop over all columns:
  for (uint8_t col = 0; col < ROC_NUMCOLS; col++) {

    // Enable this column on every configured ROC:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Col_Enable(col, true);
    }

    // Loop over all rows:
    for (uint8_t row = 0; row < ROC_NUMROWS; row++) {

      // Set the calibrate bits on every configured ROC
      // Take into account both Xtalks and Cals flags
      for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	roc_I2cAddr(roc_i2c.at(roc));
	// If masked, enable the pixel:
	if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(col, row, 15);
	roc_Pix_Cal(col, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));
      }

      // Loop over the DAC range specified:
      for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {

	// Update the DAC setting on all configured ROCS:
	for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	  roc_I2cAddr(roc_i2c.at(roc));
	  // Check if we need to correct the DAC value to be set:
	  if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
	  else roc_SetDAC(dac1register, CalibratedDAC(dac1));
	}

	// Send the triggers:
	uDelay(5);
	for (uint16_t trig = 0; trig < nTriggers; trig++) {
	  Pg_Single();
	  // Delay the next trigger, depending in the data traffic we expect:
	  uDelay(TRIGGER_DELAY*roc_i2c.size());
	}
      } // Loop over the DAC range

      // Clear the calibrate signal on every ROC configured
      for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	roc_I2cAddr(roc_i2c.at(roc));
	if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(col, row);
	roc_ClrCal();
      }
    } // Loop over all rows

    // Disable this column on every ROC configured:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Col_Enable(col, false);
    }

  } // Loop over all columns
}

void CTestboard::LoopMultiRocOnePixelDacScan(vector<uint8_t> &roc_i2c, uint8_t column, uint8_t row, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high) {

  // Enable this column on every configured ROC:
  // Set the calibrate bits on every configured ROC
  // Take into account both Xtalks and Cals flags
  for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
    roc_I2cAddr(roc_i2c.at(roc));
    if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();
    roc_Col_Enable(column, true);

    // If masked, enable the pixel:
    if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(column, row, 15);

    roc_Pix_Cal(column, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));
  }

  // Loop over the DAC range specified:
  for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {

    // Update the DAC setting on all configured ROCS:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      // Check if we need to correct the DAC value to be set:
      if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
      else roc_SetDAC(dac1register, CalibratedDAC(dac1));
    }

    // Send the triggers:
    uDelay(5);
    for (uint16_t trig = 0; trig < nTriggers; trig++) {
      Pg_Single();
      // Delay the next trigger, depending in the data traffic we expect:
      uDelay(TRIGGER_DELAY*roc_i2c.size());
    }
  } // Loop over the DAC range

  // Clear the calibrate signal on every ROC configured
  // Disable this column on every ROC configured:
  for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
    roc_I2cAddr(roc_i2c.at(roc));
    if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(column, row);
    roc_ClrCal();
    roc_Col_Enable(column, false);
  }
}

void CTestboard::LoopSingleRocAllPixelsDacScan(uint8_t roc_i2c, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high) {

  // Set the I2C output to the correct ROC:
  roc_I2cAddr(roc_i2c);
  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();

  // Loop over all columns:
  for (uint8_t col = 0; col < ROC_NUMCOLS; col++) {

    // Enable this column on the configured ROC:
    roc_Col_Enable(col, true);

    // Loop over all rows:
    for (uint8_t row = 0; row < ROC_NUMROWS; row++) {

      // If masked, enable the pixel:
      if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(col, row, 15);

      // Set the calibrate bits
      // Take into account both Xtalks and Cals flags
      roc_Pix_Cal(col, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));

      // Loop over the DAC range specified:
      for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {

	// Update the DAC setting on the ROC:
	// Check if we need to correct the DAC value to be set:
	if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
	else roc_SetDAC(dac1register, CalibratedDAC(dac1));

	// Send the triggers:
	uDelay(5);
	for (uint16_t trig = 0; trig < nTriggers; trig++) {
	  Pg_Single();
	  uDelay(TRIGGER_DELAY);
	}
      } // Loop over the DAC range

      // Clear the calibrate signal
      if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(col, row);
      roc_ClrCal();
    } // Loop over all rows

    // Disable this column on every ROC configured:
    roc_Col_Enable(col, false);

  } // Loop over all columns
}

void CTestboard::LoopSingleRocOnePixelDacScan(uint8_t roc_i2c, uint8_t column, uint8_t row, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high) {

  // Enable this column on the configured ROC:
  // Set the calibrate bits on every configured ROC
  // Take into account both Xtalks and Cals flags
  roc_I2cAddr(roc_i2c);
  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();

  roc_Col_Enable(column, true);
  // If masked, enable the pixel:
  if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(column, row, 15);
  roc_Pix_Cal(column, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));

  // Loop over the DAC range specified:
  for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {

    // Update the DAC setting on the ROC:
    // Check if we need to correct the DAC value to be set:
    if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
    else roc_SetDAC(dac1register, CalibratedDAC(dac1));

    // Send the triggers:
    uDelay(5);
    for (uint16_t trig = 0; trig < nTriggers; trig++) {
      Pg_Single();
      uDelay(TRIGGER_DELAY);
    }
  } // Loop over the DAC range

  // Clear the calibrate signal on thr ROC configured
  // Disable this column on the ROC configured:
  if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(column, row);
  roc_ClrCal();
  roc_Col_Enable(column, false);
}

// -------- Trigger Loop Functions for 2D DacDac Scans ----------------------------

void CTestboard::LoopMultiRocAllPixelsDacDacScan(vector<uint8_t> &roc_i2c, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high, uint8_t dac2register, uint8_t dac2low, uint8_t dac2high) {

  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) {
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Chip_Mask();
    }
  }

  // Loop over all columns:
  for (uint8_t col = 0; col < ROC_NUMCOLS; col++) {

    // Enable this column on every configured ROC:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Col_Enable(col, true);
    }

    // Loop over all rows:
    for (uint8_t row = 0; row < ROC_NUMROWS; row++) {

      // Set the calibrate bits on every configured ROC
      // Take into account both Xtalks and Cals flags
      for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	roc_I2cAddr(roc_i2c.at(roc));
	// If masked, enable the pixel:
	if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(col, row, 15);
	roc_Pix_Cal(col, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));
      }

      // Loop over the DAC1 range specified:
      for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {

	// Update the DAC1 setting on all configured ROCS:
	for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	  roc_I2cAddr(roc_i2c.at(roc));
	  // Check if we need to correct the DAC value to be set:
	  if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
	  else roc_SetDAC(dac1register, CalibratedDAC(dac1));
	}

	// Loop over the DAC2 range specified:
	for (size_t dac2 = dac2low; dac2 <= dac2high; dac2++) {

	  // Update the DAC2 setting on all configured ROCS:
	  for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	    roc_I2cAddr(roc_i2c.at(roc));
	    // Check if we need to correct the DAC value to be set:
	    if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac2register, dac2);
	    else roc_SetDAC(dac2register, CalibratedDAC(dac2));
	  }

	  // Send the triggers:
	  uDelay(5);
	  for (uint16_t trig = 0; trig < nTriggers; trig++) {
	    Pg_Single();
	    // Delay the next trigger, depending in the data traffic we expect:
	    uDelay(TRIGGER_DELAY*roc_i2c.size());
	  }
	} // Loop over the DAC2 range
      } // Loop over the DAC1 range

      // Clear the calibrate signal on every ROC configured
      for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	roc_I2cAddr(roc_i2c.at(roc));
	if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(col, row);
	roc_ClrCal();
      }
    } // Loop over all rows

    // Disable this column on every ROC configured:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      roc_Col_Enable(col, false);
    }

  } // Loop over all columns
}

void CTestboard::LoopMultiRocOnePixelDacDacScan(vector<uint8_t> &roc_i2c, uint8_t column, uint8_t row, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high, uint8_t dac2register, uint8_t dac2low, uint8_t dac2high) {

  // Enable this column on every configured ROC:
  // Set the calibrate bits on every configured ROC
  // Take into account both Xtalks and Cals flags
  for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
    roc_I2cAddr(roc_i2c.at(roc));
    if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();
    roc_Col_Enable(column, true);
    // If masked, enable the pixel:
    if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(column, row, 15);
    roc_Pix_Cal(column, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));
  }

  // Loop over the DAC1 range specified:
  for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {
    
    // Update the DAC1 setting on all configured ROCS:
    for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
      roc_I2cAddr(roc_i2c.at(roc));
      // Check if we need to correct the DAC value to be set:
      if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
      else roc_SetDAC(dac1register, CalibratedDAC(dac1));
    }

    // Loop over the DAC2 range specified:
    for (size_t dac2 = dac2low; dac2 <= dac2high; dac2++) {

      // Update the DAC2 setting on all configured ROCS:
      for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
	roc_I2cAddr(roc_i2c.at(roc));
	// Check if we need to correct the DAC value to be set:
	if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac2register, dac2);
	else roc_SetDAC(dac2register, CalibratedDAC(dac2));
      }

      // Send the triggers:
      uDelay(5);
      for (uint16_t trig = 0; trig < nTriggers; trig++) {
	Pg_Single();
	// Delay the next trigger, depending in the data traffic we expect:
        uDelay(TRIGGER_DELAY*roc_i2c.size());
      }
    } // Loop over the DAC2 range
  } // Loop over the DAC1 range

  // Clear the calibrate signal on every ROC configured
  // Disable this column on every ROC configured:
  for(size_t roc = 0; roc < roc_i2c.size(); roc++) {
    roc_I2cAddr(roc_i2c.at(roc));
    if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(column, row);
    roc_ClrCal();
    roc_Col_Enable(column, false);
  }
}

void CTestboard::LoopSingleRocAllPixelsDacDacScan(uint8_t roc_i2c, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high, uint8_t dac2register, uint8_t dac2low, uint8_t dac2high) {

  // Set the I2C output to the correct ROC:
  roc_I2cAddr(roc_i2c);
  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();

  // Loop over all columns:
  for (uint8_t col = 0; col < ROC_NUMCOLS; col++) {

    // Enable this column on the configured ROC:
    roc_Col_Enable(col, true);

    // Loop over all rows:
    for (uint8_t row = 0; row < ROC_NUMROWS; row++) {

      // If masked, enable the pixel:
      if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(col, row, 15);

      // Set the calibrate bits
      // Take into account both Xtalks and Cals flags
      roc_Pix_Cal(col, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));

      // Loop over the DAC1 range specified:
      for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {
    
	// Update the DAC1 setting on the configured ROC:
	// Check if we need to correct the DAC value to be set:
	if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
	else roc_SetDAC(dac1register, CalibratedDAC(dac1));

	// Loop over the DAC2 range specified:
	for (size_t dac2 = dac2low; dac2 <= dac2high; dac2++) {

	  // Update the DAC2 setting on the configured ROC:
	  // Check if we need to correct the DAC value to be set:
	  if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac2register, dac2);
	  else roc_SetDAC(dac2register, CalibratedDAC(dac2));

	  // Send the triggers:
	  uDelay(5);
	  for (uint16_t trig = 0; trig < nTriggers; trig++) {
	    Pg_Single();
	    uDelay(TRIGGER_DELAY);
	  }
	} // Loop over the DAC2 range
      } // Loop over the DAC1 range

      // Clear the calibrate signal
      if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(col, row);
      roc_ClrCal();
    } // Loop over all rows

    // Disable this column on every ROC configured:
    roc_Col_Enable(col, false);

  } // Loop over all columns
}

void CTestboard::LoopSingleRocOnePixelDacDacScan(uint8_t roc_i2c, uint8_t column, uint8_t row, uint16_t nTriggers, uint16_t flags, uint8_t dac1register, uint8_t dac1low, uint8_t dac1high, uint8_t dac2register, uint8_t dac2low, uint8_t dac2high) {

  // Enable this column on the configured ROC:
  // Set the calibrate bits on every configured ROC
  // Take into account both Xtalks and Cals flags
  roc_I2cAddr(roc_i2c);
  // If FLAG_FORCE_MASKED is set, mask the chip:
  if(flags&FLAG_FORCE_MASKED) roc_Chip_Mask();

  roc_Col_Enable(column, true);
  // If masked, enable the pixel:
  if(flags&FLAG_FORCE_MASKED) roc_Pix_Trim(column, row, 15);
  roc_Pix_Cal(column, GetXtalkRow(row,(flags&FLAG_XTALK)), (flags&FLAG_CALS));

  // Loop over the DAC range specified:
  for (size_t dac1 = dac1low; dac1 <= dac1high; dac1++) {
    
    // Update the DAC1 setting on the configured ROC:
    // Check if we need to correct the DAC value to be set:
    if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac1register, dac1);
    else roc_SetDAC(dac1register, CalibratedDAC(dac1));

    // Loop over the DAC2 range specified:
    for (size_t dac2 = dac2low; dac2 <= dac2high; dac2++) {

      // Update the DAC2 setting on the configured ROC:
      // Check if we need to correct the DAC value to be set:
      if(flags&FLAG_DISABLE_DACCAL) roc_SetDAC(dac2register, dac2);
      else roc_SetDAC(dac2register, CalibratedDAC(dac2));

      // Send the triggers:
      uDelay(5);
      for (uint16_t trig = 0; trig < nTriggers; trig++) {
	Pg_Single();
	uDelay(TRIGGER_DELAY);
      }
    } // Loop over the DAC2 range
  } // Loop over the DAC1 range

  // Clear the calibrate signal on the ROC configured
  // Disable this column on the ROC configured:
  if(flags&FLAG_FORCE_MASKED) roc_Pix_Mask(column, row);
  roc_ClrCal();
  roc_Col_Enable(column, false);
}
