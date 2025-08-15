// Copyright (c) 2025, TheByteSlayer, Lithium Cache

use super::Config;

use std::io;

impl Config {
    pub fn recover_from_corruption(corrupted_content: &str, config_path: &str) -> io::Result<Config> {
        let recovered_config = Self::attempt_recovery(corrupted_content);
        Self::write_healed_config(&recovered_config, config_path)?;
        Ok(recovered_config)
    }

    fn attempt_recovery(corrupted_content: &str) -> Config {
        let mut recovered_config = Config::default();
        
        if let Some(ip_match) = extract_json_string_value(corrupted_content, "ip") {
            if !ip_match.is_empty() && ip_match != "null" {
                recovered_config.bind.ip = ip_match;
            }
        }
        
        if let Some(port_str) = extract_json_number_value(corrupted_content, "port") {
            if let Ok(port) = port_str.parse::<u16>() {
                if port > 0 {
                    recovered_config.bind.port = port;
                }
            }
        }
        
        recovered_config
    }
}

fn extract_json_string_value(content: &str, key: &str) -> Option<String> {
    let patterns = [
        format!(r#""{}"\s*:\s*"([^"]+)""#, key),
        format!(r#"'{}'\s*:\s*'([^']+)'"#, key),
        format!(r#"{}:\s*"([^"]+)""#, key),
        format!(r#"{}:\s*'([^']+)'"#, key),
    ];
    
    for pattern in &patterns {
        if let Ok(regex) = regex::Regex::new(pattern) {
            if let Some(captures) = regex.captures(content) {
                if let Some(value) = captures.get(1) {
                    return Some(value.as_str().to_string());
                }
            }
        }
    }
    
    None
}

fn extract_json_number_value(content: &str, key: &str) -> Option<String> {
    let patterns = [
        format!(r#""{}"\s*:\s*(\d+)"#, key),
        format!(r#"'{}'\s*:\s*(\d+)"#, key),
        format!(r#"{}:\s*(\d+)"#, key),
    ];
    
    for pattern in &patterns {
        if let Ok(regex) = regex::Regex::new(pattern) {
            if let Some(captures) = regex.captures(content) {
                if let Some(value) = captures.get(1) {
                    return Some(value.as_str().to_string());
                }
            }
        }
    }
    
    None
}
