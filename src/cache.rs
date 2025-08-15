// Copyright (c) 2025, TheByteSlayer, Lithium Cache

use std::collections::HashMap;
use std::sync::{Arc, RwLock};

#[derive(Debug, Clone)]
pub struct Cache {
    data: Arc<RwLock<HashMap<String, String>>>,
}

impl Cache {
    pub fn new() -> Self {
        Cache {
            data: Arc::new(RwLock::new(HashMap::new())),
        }
    }

    pub fn set(&self, key: String, value: String) -> Result<(), String> {
        match self.data.write() {
            Ok(mut data) => {
                data.insert(key, value);
                Ok(())
            }
            Err(e) => Err(format!("Cache write error: {}", e)),
        }
    }

    pub fn get(&self, key: &str) -> Result<Option<String>, String> {
        match self.data.read() {
            Ok(data) => Ok(data.get(key).cloned()),
            Err(e) => Err(format!("Cache read error: {}", e)),
        }
    }

    pub fn delete(&self, key: &str) -> Result<bool, String> {
        match self.data.write() {
            Ok(mut data) => Ok(data.remove(key).is_some()),
            Err(e) => Err(format!("Cache write error: {}", e)),
        }
    }

    pub fn keys(&self) -> Result<Vec<String>, String> {
        match self.data.read() {
            Ok(data) => Ok(data.keys().cloned().collect()),
            Err(e) => Err(format!("Cache read error: {}", e)),
        }
    }
}

impl Default for Cache {
    fn default() -> Self {
        Self::new()
    }
}