use std::env;
use std::cmp::Ordering;
use serde::{Deserialize, Serialize};
use std::fs::File;
use colored::*;
use log::{error, info, warn};

#[derive(Serialize, Deserialize, Default)]
struct Setting {
    welcome_message: String,
}

#[derive(Serialize, Deserialize, PartialEq, Eq, Ord)]
struct Task {
    done: bool,
    name: String,
}

impl PartialOrd for Task {
    fn partial_cmp(&self, other:&Self) -> Option<Ordering> {
        if self.done && !other.done {
            Some(Ordering::Less)
        } else if !self.done && other.done {
            Some(Ordering::Greater)
        } else {
            Some(self.name.cmp(&other.name))
        }
    }
}

#[derive(Serialize, Deserialize)]
struct Data {
    #[serde(default)]
    settings: Setting,
    tasks: Vec<Task>,
}

fn print_tasks(data: &Data) {
    if !data.settings.welcome_message.is_empty() {
        println!("{}", data.settings.welcome_message.blue().italic().bold());
    }
    for (pos, task) in data.tasks.iter().enumerate() { 
        let s = format!("{} [{}] {}", pos, if task.done {'x'} else {' '}, task.name);
        if task.done {
            println!("{}", s.green());
        } else {
            println!("{}", s.bold().bright_red());
        }
    }
}

fn save_tasks(data: &Data, path: String) -> Result<(), std::io::Error> {
    std::fs::write(
        path,
        serde_json::to_string_pretty(&data).unwrap(),
    )?;
    Ok(())
}

fn main() -> std::result::Result<(), Box<dyn std::error::Error>> {
    env_logger::init();

    let path = match env::var("HOME") {
        Err(_) => "./".to_string(),
        Ok(p) => p,
    } + "/plc.dat";
    let f = File::open(&path)?;

    let mut data: Data = serde_json::from_reader(f)?;
    
    let args: Vec<String> = env::args().collect();

    if args.len() >= 2 {
        match args[1].as_str() {
            "add" => {
                let name = &args[2..args.len()].join(" ");
                data.tasks.push(Task {
                    done: false,
                    name: name.to_string()
                });
            },
            "rm" => {
                let mut indices = args[2..args.len()]
                    .to_vec()
                    .iter()
                    .filter_map(|x| {
                        if let Ok(r) = x.parse::<usize>() {
                            if r < data.tasks.len() {
                                Some(r)
                            } else {
                                None
                            }
                        }
                        else {
                            None
                        }
                    })
                    .collect::<Vec<_>>();
                indices.sort_unstable();
                for it in indices.iter().rev() {
                    data.tasks.remove(*it);
                }
            },
            "do" => {
                for arg in &args[2..args.len()] {
                    if let Ok(pos) = arg.parse::<usize>() {
                        if pos < args.len() {
                            data.tasks[pos].done ^= true;
                        } else {
                            warn!("do: out of bound index");
                        }
                    } else {
                        warn!("do: invalid index");
                    }
                }
            },
            "clean" => {
                data.tasks.retain(|task| { !task.done });
            },
            "sort" => {}
            "set" => {
                if args.len() < 4 {
                    info!("plc set key value");
                } else {
                    match args[2].as_str() {
                        "welcome_message" => {
                            data.settings.welcome_message = args[3].to_string();
                        }
                        _ => {}
                    }
                }
            }
            _ => {}
        }
    }

    data.tasks.sort_unstable();
    print_tasks(&data);

    if let Ok(_) = save_tasks(&data, path) {
    } else {
        error!("Failed to save file");
    }

    Ok(())
}
