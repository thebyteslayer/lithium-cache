// Copyright (c) 2025, TheByteSlayer, Lithium Cache

mod recovery;
mod healing;

use serde::{Deserialize, Serialize};
use std::fs;
use std::io;
use std::path::Path;

#[derive(Debug, Serialize, Deserialize, Clone)]
#[serde(rename_all = "lowercase")]
pub enum Mode {
    Memory,
    Disk,
    Hybrid,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct BindConfig {
    #[serde(default = "default_ip")]
    pub ip: String,
    #[serde(default = "default_port")]
    pub port: u16,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Config {
    #[serde(default = "default_mode")]
    pub mode: Mode,
    #[serde(default)]
    pub bind: BindConfig,
    #[serde(default = "default_silent")]
    pub silent: bool,
    #[serde(default = "default_logging")]
    pub logging: bool,
}

fn default_ip() -> String {
    "0.0.0.0".to_string()
}

fn default_port() -> u16 {
    1227
}

fn default_mode() -> Mode {
    Mode::Memory
}

fn default_silent() -> bool {
    false
}

fn default_logging() -> bool {
    false
}

impl Default for Config {
    fn default() -> Self {
        Config {
            mode: default_mode(),
            bind: BindConfig {
                ip: default_ip(),
                port: default_port(),
            },
            silent: default_silent(),
            logging: default_logging(),
        }
    }
}

impl Default for BindConfig {
    fn default() -> Self {
        BindConfig {
            ip: default_ip(),
            port: default_port(),
        }
    }
}

impl Config {
    pub fn load_or_create() -> io::Result<Self> {
        let config_path = "lithium.json";
        
        if Path::new(config_path).exists() {
            let content = fs::read_to_string(config_path)?;
            
            match serde_json::from_str(&content) {
                Ok(config) => {
                    Self::heal_and_save(&config, config_path, &content)?;
                    Ok(config)
                }
                Err(_) => {
                    Self::recover_from_corruption(&content, config_path)
                }
            }
        } else {
            Self::create_default_config(config_path)
        }
    }
}
