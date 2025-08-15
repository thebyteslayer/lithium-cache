// Copyright (c) 2025, TheByteSlayer, Lithium Cache

use rand::Rng;
use regex::Regex;
use serde::{Deserialize, Serialize};
use std::fs;
use std::io;
use std::path::Path;

#[derive(Debug, Serialize, Deserialize)]
pub struct TokenConfig {
    pub token: String,
}

impl TokenConfig {
    pub fn is_valid_token(token: &str) -> bool {
        let pattern = Regex::new(r"^[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}$").unwrap();
        pattern.is_match(token)
    }
    
    pub fn generate_token() -> String {
        let mut rng = rand::thread_rng();
        let mut token_parts = Vec::new();
        
        for _ in 0..4 {
            let part: u16 = rng.r#gen();
            token_parts.push(format!("{:04x}", part));
        }
        
        token_parts.join("-")
    }
    
    pub fn new() -> Self {
        TokenConfig {
            token: Self::generate_token(),
        }
    }
    
    pub fn load_or_create() -> io::Result<Self> {
        let token_path = "token.json";
        
        if Path::new(token_path).exists() {
            let content = fs::read_to_string(token_path)?;
            match serde_json::from_str::<TokenConfig>(&content) {
                Ok(token_config) => {
                    if Self::is_valid_token(&token_config.token) {
                        Ok(token_config)
                    } else {
                        let new_token_config = Self::new();
                        new_token_config.save(token_path)?;
                        Ok(new_token_config)
                    }
                }
                Err(_) => {
                    let new_token_config = Self::new();
                    new_token_config.save(token_path)?;
                    Ok(new_token_config)
                }
            }
        } else {
            let new_token_config = Self::new();
            new_token_config.save(token_path)?;
            Ok(new_token_config)
        }
    }
    
    pub fn save(&self, path: &str) -> io::Result<()> {
        let json_content = serde_json::to_string_pretty(self)
            .map_err(|e| io::Error::new(io::ErrorKind::InvalidData, e))?;
        fs::write(path, json_content)
    }
}

impl Default for TokenConfig {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use regex::Regex;
    
    #[test]
    fn test_token_format() {
        let token = TokenConfig::generate_token();
        let pattern = Regex::new(r"^[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}$").unwrap();
        assert!(pattern.is_match(&token), "Token '{}' doesn't match expected format", token);
    }
    
    #[test]
    fn test_token_uniqueness() {
        let token1 = TokenConfig::generate_token();
        let token2 = TokenConfig::generate_token();
        assert_ne!(token1, token2, "Generated tokens should be unique");
    }
    
    #[test]
    fn test_token_validation() {
        assert!(TokenConfig::is_valid_token("1a2b-3c4d-5e6f-7f80"));
        assert!(TokenConfig::is_valid_token("0000-0000-0000-0000"));
        assert!(TokenConfig::is_valid_token("ffff-ffff-ffff-ffff"));
        
        assert!(!TokenConfig::is_valid_token("1a2b-3c4d-5e6f"));
        assert!(!TokenConfig::is_valid_token("1a2b-3c4d-5e6f-7f80-1234"));
        assert!(!TokenConfig::is_valid_token("1A2B-3C4D-5E6F-7F80"));
        assert!(!TokenConfig::is_valid_token("1a2b_3c4d_5e6f_7f80"));
        assert!(!TokenConfig::is_valid_token("1a2g-3c4d-5e6f-7f80"));
        assert!(!TokenConfig::is_valid_token(""));
        assert!(!TokenConfig::is_valid_token("not-a-token"));
    }
}
