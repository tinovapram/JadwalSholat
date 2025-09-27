/*
 * Debug Utilities Implementation
 */

#include "global.h"

void debugPrint(const String& message) {
  Serial.print(F("[DEBUG] "));
  Serial.print(message);
}

void debugPrintln(const String& message) {
  Serial.print(F("[DEBUG] "));
  Serial.println(message);
}