mod check;
mod course;
mod watch;

use std::process::ExitCode;

use check::{GREEN, Outcome, RED, RESET};
use course::Course;

pub type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;

const HELP: &str = "\
ccourse: learn C in the terminal

Usage:
  ccourse           start the course (watch mode)
  ccourse verify    for course authors: check every exercise and solution
  ccourse --help    show this text

Run it from the course directory, keep it open, and edit the exercise file
it names in your editor. Each time you save, the exercise is checked again.";

fn main() -> ExitCode {
    let result = match std::env::args().nth(1).as_deref() {
        None => Course::load().and_then(|course| watch::run(&course)),
        Some("verify") => Course::load().and_then(|course| verify(&course)),
        Some("-h" | "--help") => {
            println!("{HELP}");
            Ok(())
        }
        Some(other) => Err(format!("unknown argument '{other}' (try --help)").into()),
    };
    match result {
        Ok(()) => ExitCode::SUCCESS,
        Err(e) => {
            eprintln!("ccourse: {e}");
            ExitCode::FAILURE
        }
    }
}

/// Every original exercise must fail and every reference solution must pass.
fn verify(course: &Course) -> Result<()> {
    let mut problems = 0;
    for ex in &course.exercises {
        let mut faults = Vec::new();
        if let Outcome::Passed(_) = check::check(course, ex, &course.pristine(ex)) {
            faults.push("the untouched exercise already passes".to_string());
        }
        if let Outcome::Failed { title, detail } = check::check(course, ex, &course.solution(ex)) {
            faults.push(format!("the solution fails: {title}\n{detail}"));
        }
        if ex.hints.is_empty() {
            faults.push("no hints".to_string());
        }
        if faults.is_empty() {
            println!("{GREEN}ok{RESET}    {}/{}", ex.dir, ex.name);
        } else {
            problems += 1;
            println!("{RED}FAIL{RESET}  {}/{}", ex.dir, ex.name);
            for fault in faults {
                println!("      {}", fault.replace('\n', "\n      "));
            }
        }
    }
    if problems > 0 {
        return Err(format!("{problems} of {} exercises have problems", course.exercises.len()).into());
    }
    println!("\nAll {} exercises verified.", course.exercises.len());
    Ok(())
}
