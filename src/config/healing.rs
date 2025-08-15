// Copyright (c) 2025, TheByteSlayer, Lithium Cache

use super::Config;
use std::fs;
use std::io;

impl Config {
    pub fn heal_and_save(config: &Config, config_path: &str, original_content: &str) -> io::Result<()> {
        let healed_json = serde_json::to_string_pretty(config)
            .map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
        
        if original_content.trim() != healed_json.trim() {
            fs::write(config_path, healed_json)?;
            println!("Auto-healed missing configuration values");
        }
        
        Ok(())
    }
    
    pub fn write_healed_config(config: &Config, config_path: &str) -> io::Result<()> {
        let json_content = serde_json::to_string_pretty(config)
            .map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
        fs::write(config_path, json_content)?;
        println!("Attempted to recover valid values from corrupted configuration");
        Ok(())
    }
    
    pub fn create_default_config(config_path: &str) -> io::Result<Config> {
        let default_config = Config::default();
        let json_content = serde_json::to_string_pretty(&default_config)
            .map_err(|e| io::Error::new(io::ErrorKind::Other, e))?;
        fs::write(config_path, json_content)?;
        println!("Created new configuration file: {}", config_path);
        Ok(default_config)
    }
}
