//! Compiling an exercise, running it, and deciding whether it passes.

use std::fs;
use std::io::{self, Read, Write};
use std::path::{Path, PathBuf};
use std::process::{Command, Stdio};
use std::thread;
use std::time::{Duration, Instant};

use crate::course::{Course, Exercise, Kind, Test};

pub const RESET: &str = "\x1b[0m";
pub const BOLD: &str = "\x1b[1m";
pub const DIM: &str = "\x1b[2m";
pub const RED: &str = "\x1b[31m";
pub const GREEN: &str = "\x1b[32m";
pub const YELLOW: &str = "\x1b[33m";
pub const CYAN: &str = "\x1b[36m";

/// A runaway program is cut off after this much output or this much time.
const OUTPUT_LIMIT: usize = 64 * 1024;
const TIME_LIMIT: Duration = Duration::from_secs(5);

pub enum Outcome {
    /// Carries what the program printed.
    Passed(String),
    Failed { title: String, detail: String },
}

fn failed(title: &str, detail: impl Into<String>) -> Outcome {
    Outcome::Failed { title: title.to_string(), detail: detail.into() }
}

pub fn check(course: &Course, ex: &Exercise, source: &Path) -> Outcome {
    let bin = match compile(course, ex, source) {
        Ok(bin) => bin,
        Err(outcome) => return outcome,
    };
    if ex.kind == Kind::Predict {
        return check_prediction(&bin, source);
    }

    let mut first_output = String::new();
    for (i, test) in ex.tests.iter().enumerate() {
        let run = match run(&bin, test) {
            Ok(run) => run,
            Err(e) => return failed("Could not run the program", e.to_string()),
        };
        if let Some(problem) = run.compare(test) {
            let mut detail = String::new();
            if ex.tests.len() > 1 {
                detail += &format!("{DIM}Test {} of {}{RESET}\n", i + 1, ex.tests.len());
            }
            if !test.args.is_empty() {
                detail += &format!("Run with arguments: {}\n", test.args.join(" "));
            }
            if !test.stdin.is_empty() {
                detail += &format!("Given this input:\n{}", block(&test.stdin));
            }
            return failed("The program compiles, but does not behave as required", detail + &problem);
        }
        if i == 0 {
            first_output = run.stdout;
        }
    }
    Outcome::Passed(first_output)
}

fn compile(course: &Course, ex: &Exercise, source: &Path) -> Result<PathBuf, Outcome> {
    let bin_dir = course.bin_dir();
    if let Err(e) = fs::create_dir_all(&bin_dir) {
        return Err(failed("Could not create the build directory", e.to_string()));
    }
    let bin = bin_dir.join(&ex.name);
    let compiler = std::env::var("CC").unwrap_or_else(|_| "cc".to_string());

    let mut cmd = Command::new(&compiler);
    cmd.current_dir(&course.root)
        .args(["-std=c17", "-Wall", "-Wextra", "-Wpedantic", "-g", "-fdiagnostics-color=always"]);
    if ex.sanitize {
        cmd.args(["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]);
    }
    cmd.arg(course.relative(source)).arg("-o").arg(&bin);

    let output = match cmd.output() {
        Ok(output) => output,
        Err(e) => {
            return Err(failed(
                "Could not start the C compiler",
                format!("Tried to run `{compiler}`: {e}\nInstall clang or gcc, or set the CC environment variable."),
            ));
        }
    };
    let messages = String::from_utf8_lossy(&output.stderr).into_owned();
    if !output.status.success() {
        return Err(failed("The program does not compile", messages));
    }
    if !messages.trim().is_empty() {
        return Err(failed("The program compiles, but with warnings. Fix those too", messages));
    }
    Ok(bin)
}

struct Run {
    stdout: String,
    stderr: String,
    exit: Option<i32>,
    signal: Option<i32>,
    timed_out: bool,
}

fn read_capped(reader: impl Read) -> String {
    let mut buf = Vec::new();
    let _ = reader.take(OUTPUT_LIMIT as u64).read_to_end(&mut buf);
    String::from_utf8_lossy(&buf).into_owned()
}

fn run(bin: &Path, test: &Test) -> io::Result<Run> {
    let mut child = Command::new(bin)
        .args(&test.args)
        // Silences a macOS allocator notice that AddressSanitizer triggers.
        .env("MallocNanoZone", "0")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .spawn()?;

    let mut stdin = child.stdin.take().expect("stdin is piped");
    let input = test.stdin.clone();
    thread::spawn(move || {
        let _ = stdin.write_all(input.as_bytes());
    });
    let stdout = child.stdout.take().expect("stdout is piped");
    let stderr = child.stderr.take().expect("stderr is piped");
    let stdout = thread::spawn(move || read_capped(stdout));
    let stderr = thread::spawn(move || read_capped(stderr));

    let started = Instant::now();
    let mut timed_out = false;
    let status = loop {
        if let Some(status) = child.try_wait()? {
            break status;
        }
        if started.elapsed() > TIME_LIMIT {
            timed_out = true;
            let _ = child.kill();
            break child.wait()?;
        }
        thread::sleep(Duration::from_millis(5));
    };

    #[cfg(unix)]
    let signal = std::os::unix::process::ExitStatusExt::signal(&status);
    #[cfg(not(unix))]
    let signal = None;

    Ok(Run {
        stdout: stdout.join().unwrap_or_default(),
        stderr: stderr.join().unwrap_or_default(),
        exit: status.code(),
        signal,
        timed_out,
    })
}

impl Run {
    /// Describes how this run differs from what the test expects, if it does.
    fn compare(&self, test: &Test) -> Option<String> {
        if self.timed_out {
            return Some(format!(
                "The program was still running after {} seconds and was stopped.\nIs there a loop that never ends?\n",
                TIME_LIMIT.as_secs()
            ));
        }
        if self.stdout.len() >= OUTPUT_LIMIT {
            return Some("The program printed far too much and was stopped.\nIs there a loop that never ends?\n".to_string());
        }

        let mut problem = String::new();
        match (self.exit, self.signal) {
            (Some(code), _) if code == test.exit => {}
            (Some(code), _) => {
                problem += &format!("Expected exit code {}, but the program exited with {code}.\n", test.exit);
            }
            (None, signal) => {
                let name = match signal {
                    Some(6) => " (abort)",
                    Some(8) => " (arithmetic error, such as division by zero)",
                    Some(10) => " (bus error: invalid memory access)",
                    Some(11) => " (segmentation fault: invalid memory access)",
                    _ => "",
                };
                let number = signal.map_or("unknown".to_string(), |s| s.to_string());
                problem += &format!("The program crashed: killed by signal {number}{name}.\n");
            }
        }
        // A crash loses whatever output was still buffered, so comparing it would mislead.
        if let Some(expected) = &test.stdout
            && self.exit.is_some()
            && normalise(expected) != normalise(&self.stdout)
        {
            problem += &format!("Expected output:\n{}", block(expected));
            problem += &format!("Your program printed:\n{}", block(&self.stdout));
        }
        if problem.is_empty() {
            return None;
        }
        if !self.stderr.trim().is_empty() {
            problem += &format!("Error output of the program:\n{}", block(&without_memory_dump(&self.stderr)));
        }
        Some(problem)
    }
}

fn check_prediction(bin: &Path, source: &Path) -> Outcome {
    let test = Test { args: vec![], stdin: String::new(), stdout: None, exit: 0 };
    let run = match run(bin, &test) {
        Ok(run) => run,
        Err(e) => return failed("Could not run the program", e.to_string()),
    };
    let text = fs::read_to_string(source).unwrap_or_default();
    let Some(prediction) = prediction(&text) else {
        return failed(
            "The PREDICT markers are missing",
            "The file must end with a line `/* PREDICT`, your prediction, then a line `*/`.\nPress x to reset the file if you removed them.\n",
        );
    };
    let predicted = trimmed(&prediction);
    let actual = trimmed(&run.stdout);
    if predicted.is_empty() {
        return failed(
            "Waiting for your prediction",
            "Read the code, then write exactly what it prints between the\n`/* PREDICT` and `*/` lines at the bottom of the file, and save.\n",
        );
    }
    if predicted == actual {
        return Outcome::Passed(run.stdout);
    }
    let detail = match predicted.iter().zip(&actual).position(|(p, a)| p != a) {
        Some(i) => format!("Line {} of your prediction is not what the program prints.\n", i + 1),
        None if predicted.len() < actual.len() => {
            format!("Correct so far, but the program prints more lines than your {}.\n", predicted.len())
        }
        None => format!("The program prints only {} line(s); your prediction has more.\n", actual.len()),
    };
    failed("The prediction does not match", detail)
}

/// The text between the `/* PREDICT` and `*/` marker lines.
fn prediction(source: &str) -> Option<String> {
    let mut lines = source.lines().skip_while(|line| line.trim() != "/* PREDICT");
    lines.next()?;
    let body: Vec<&str> = lines.take_while(|line| line.trim() != "*/").collect();
    Some(body.join("\n"))
}

/// Output as lines, ignoring trailing spaces and trailing blank lines.
fn normalise(text: &str) -> Vec<&str> {
    let mut lines: Vec<&str> = text.lines().map(str::trim_end).collect();
    while lines.last().is_some_and(|line| line.is_empty()) {
        lines.pop();
    }
    lines
}

/// Like `normalise`, but also forgiving of indentation and leading blank lines.
fn trimmed(text: &str) -> Vec<&str> {
    normalise(text.trim_start_matches(['\n', '\r'])).into_iter().map(str::trim).collect()
}

/// Drops the raw memory map that ends an AddressSanitizer report.
fn without_memory_dump(report: &str) -> String {
    report.lines().take_while(|line| !line.starts_with("Shadow bytes")).map(|line| format!("{line}\n")).collect()
}

/// Sets text off from the surrounding explanation.
fn block(text: &str) -> String {
    if text.is_empty() {
        return format!("  {DIM}│ (nothing){RESET}\n");
    }
    text.lines().map(|line| format!("  {DIM}│{RESET} {line}\n")).collect()
}
