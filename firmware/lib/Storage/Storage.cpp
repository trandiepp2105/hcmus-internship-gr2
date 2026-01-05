/**
 * @file Storage.cpp
 * @brief Implementation của Storage Driver
 */

#include "Storage.h"

// ==================== Constructor & Destructor ====================

Storage::Storage() : _initialized(false) {
}

Storage::~Storage() {
    if (_initialized) {
        end();
    }
}

// ==================== Initialization ====================

bool Storage::begin(const char* namespaceName, bool readOnly) {
    if (_initialized) {
        end();
    }

    bool success = _preferences.begin(namespaceName, readOnly);
    
    if (success) {
        _initialized = true;
    }
    
    return success;
}

void Storage::end() {
    if (_initialized) {
        _preferences.end();
        _initialized = false;
    }
}

bool Storage::isInitialized() {
    return _initialized;
}

// ==================== Integer Operations ====================

bool Storage::putInt(const char* key, int32_t value) {
    if (!isInitialized()) return false;
    
    return _preferences.putInt(key, value) > 0;
}

int32_t Storage::getInt(const char* key, int32_t defaultValue) {
    if (!isInitialized()) return defaultValue;
    
    return _preferences.getInt(key, defaultValue);
}

// ==================== Float Operations ====================

bool Storage::putFloat(const char* key, float value) {
    if (!isInitialized()) return false;
    
    return _preferences.putFloat(key, value) > 0;
}

float Storage::getFloat(const char* key, float defaultValue) {
    if (!isInitialized()) return defaultValue;
    
    return _preferences.getFloat(key, defaultValue);
}

// ==================== Double Operations ====================

bool Storage::putDouble(const char* key, double value) {
    if (!isInitialized()) return false;
    
    return _preferences.putDouble(key, value) > 0;
}

double Storage::getDouble(const char* key, double defaultValue) {
    if (!isInitialized()) return defaultValue;
    
    return _preferences.getDouble(key, defaultValue);
}

// ==================== Boolean Operations ====================

bool Storage::putBool(const char* key, bool value) {
    if (!isInitialized()) return false;
    
    return _preferences.putBool(key, value) > 0;
}

bool Storage::getBool(const char* key, bool defaultValue) {
    if (!isInitialized()) return defaultValue;
    
    return _preferences.getBool(key, defaultValue);
}

// ==================== String Operations ====================

bool Storage::putString(const char* key, const String& value) {
    if (!isInitialized()) return false;
    
    return _preferences.putString(key, value) > 0;
}

String Storage::getString(const char* key, const String& defaultValue) {
    if (!isInitialized()) return defaultValue;
    
    return _preferences.getString(key, defaultValue);
}

// ==================== Utility Operations ====================

bool Storage::isKeyExist(const char* key) {
    if (!isInitialized()) return false;
    
    return _preferences.isKey(key);
}

bool Storage::remove(const char* key) {
    if (!isInitialized()) return false;
    
    return _preferences.remove(key);
}

bool Storage::clear() {
    if (!isInitialized()) return false;
    
    return _preferences.clear();
}

size_t Storage::freeEntries() {
    if (!isInitialized()) return 0;
    
    return _preferences.freeEntries();
}
