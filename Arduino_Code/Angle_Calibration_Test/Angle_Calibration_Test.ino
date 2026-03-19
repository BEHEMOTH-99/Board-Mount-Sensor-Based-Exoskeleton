// Pins for the analog inputs (matching the Sender code)
const int potPin1 = A0;
const int potPin2 = A1;
const int potPin3 = A2;

// --- CALIBRATION SETTINGS ---
// Default starting raw ADC points for the multi-point calibration.
// These are rough guesses (assuming 360 degree pot) that will be overwritten 
// when you type "zero", "45", or "90".
float pot1_0 = 0.0,   pot1_45 = 512.0, pot1_90 = 1024.0;
float pot2_0 = 0.0,   pot2_45 = 512.0, pot2_90 = 1024.0;
float pot3_0 = 0.0,   pot3_45 = 512.0, pot3_90 = 1024.0;

bool is45Set = false;
bool is90Set = false;

void setup() {
  Serial.begin(115200);
  
  // REQUIRED for S3 Native USB to show Serial output
  delay(1000);
  Serial.println("Starting Angle Check...");
  Serial.println("--- MULTI-POINT CALIBRATION ACTIVE ---");
  Serial.println("Type 'zero' to set current position as 0 degrees");
  Serial.println("Type '45' to set current position as 45 degrees");
  Serial.println("Type '90' to set current position as 90 degrees");
}

// Float version of Arduino's map() for better precision
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  if (in_max == in_min) return out_min; // Prevent divide by zero
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Calculates piecewise linear interpolation for linkage non-linearity
float calcLinkageAngle(float adc, float zeroAdc, float adc45, float adc90) {
  float diff45 = adc45 - zeroAdc; 
  if (diff45 == 0) return 0.0; // Prevent divide by zero if uncalibrated
  
  float diffX = adc - zeroAdc;    
  
  // If the current reading is proportionally past the 45 degree point 
  // towards 90, we use the 45-to-90 mapping line. Otherwise, the 0-to-45 line.
  if ((diffX / diff45) <= 1.0) {
    return mapFloat(adc, zeroAdc, adc45, 0.0, 45.0);
  } else {
    return mapFloat(adc, adc45, adc90, 45.0, 90.0);
  }
}

void loop() {
  // Read analog inputs from the potentiometers
  int raw1 = analogRead(potPin1);
  int raw2 = analogRead(potPin2);
  int raw3 = analogRead(potPin3);

  // Apply Exponential Moving Average (EMA) filter
  float alpha = 0.15; // Lower = smoother but slower response. 
  
  static float filtered1 = raw1; // initialize with first reading
  static float filtered2 = raw2;
  static float filtered3 = raw3;

  filtered1 = (alpha * raw1) + ((1.0 - alpha) * filtered1);
  filtered2 = (alpha * raw2) + ((1.0 - alpha) * filtered2);
  filtered3 = (alpha * raw3) + ((1.0 - alpha) * filtered3);

  // Check if there is a message from the Serial Monitor
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    message.trim(); // Remove any extra spaces or hidden newline characters
    
    if (message.equalsIgnoreCase("zero")) {
      pot1_0 = filtered1; pot2_0 = filtered2; pot3_0 = filtered3;
      is45Set = false; is90Set = false; // Reset the higher bounds tracker
      Serial.println("\n--- 0 DEGREE POINT SET ---");
    } 
    else if (message.equalsIgnoreCase("45")) {
      pot1_45 = filtered1; pot2_45 = filtered2; pot3_45 = filtered3;
      is45Set = true;
      
      // If 90 hasn't been calibrated yet, logically guess/extrapolate it so the math doesn't break
      if (!is90Set) { 
        pot1_90 = mapFloat(90.0, 0.0, 45.0, pot1_0, pot1_45);
        pot2_90 = mapFloat(90.0, 0.0, 45.0, pot2_0, pot2_45);
        pot3_90 = mapFloat(90.0, 0.0, 45.0, pot3_0, pot3_45);
      }
      Serial.println("\n--- 45 DEGREE POINT SET ---");
    }
    else if (message.equalsIgnoreCase("90")) {
      pot1_90 = filtered1; pot2_90 = filtered2; pot3_90 = filtered3;
      is90Set = true;
      
      // If 45 was skipped, gracefully turn the piecewise system into a normal direct linear mapping!
      if (!is45Set) { 
        pot1_45 = mapFloat(45.0, 0.0, 90.0, pot1_0, pot1_90);
        pot2_45 = mapFloat(45.0, 0.0, 90.0, pot2_0, pot2_90);
        pot3_45 = mapFloat(45.0, 0.0, 90.0, pot3_0, pot3_90);
      }
      Serial.println("\n--- 90 DEGREE POINT SET ---");
    }
  }

  // Calculate the final piecewise angles based on your calibration points
  float angle1 = calcLinkageAngle(filtered1, pot1_0, pot1_45, pot1_90);
  float angle2 = calcLinkageAngle(filtered2, pot2_0, pot2_45, pot2_90);
  float angle3 = calcLinkageAngle(filtered3, pot3_0, pot3_45, pot3_90);

  // Print the angles to the Serial monitor
  // Formatted like this so it works nicely with the Arduino IDE Serial Plotter
  Serial.print("Angle1:");
  Serial.print(angle1);
  Serial.print("\tAngle2:");
  Serial.print(angle2);
  Serial.print("\tAngle3:");
  Serial.println(angle3);

  // Delay for a stable framerate (e.g., 20ms = 50Hz)
  delay(20);
}
