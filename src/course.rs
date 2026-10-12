//! The course on disk: the exercise list, where files live, and saved progress.

use std::fs;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};

use crate::Result;

#[derive(Deserialize, Clone, Copy, PartialEq)]
#[serde(rename_all = "lowercase")]
pub enum Kind {
    Fix,
    Complete,
    Predict,
    Debug,
    Build,
}

impl Kind {
    pub fn label(self) -> &'static str {
        match self {
            Kind::Fix => "fix",
            Kind::Complete => "complete",
            Kind::Predict => "predict",
            Kind::Debug => "debug",
            Kind::Build => "build",
        }
    }
}

#[derive(Deserialize)]
pub struct Test {
    #[serde(default)]
    pub args: Vec<String>,
    #[serde(default)]
    pub stdin: String,
    /// Expected standard output; not checked when absent.
    pub stdout: Option<String>,
    #[serde(default)]
    pub exit: i32,
}

#[derive(Deserialize)]
pub struct Exercise {
    pub name: String,
    pub dir: String,
    pub kind: Kind,
    /// Build with AddressSanitizer and UndefinedBehaviorSanitizer.
    #[serde(default)]
    pub sanitize: bool,
    #[serde(default)]
    pub hints: Vec<String>,
    #[serde(default)]
    pub tests: Vec<Test>,
}

impl Exercise {
    fn file(&self) -> PathBuf {
        Path::new(&self.dir).join(format!("{}.c", self.name))
    }
}

#[derive(Deserialize)]
struct Info {
    exercises: Vec<Exercise>,
}

pub struct Course {
    pub root: PathBuf,
    pub exercises: Vec<Exercise>,
}

impl Course {
    /// Finds `course/info.toml` in the current directory or any parent.
    pub fn load() -> Result<Course> {
        let cwd = std::env::current_dir()?;
        let root = cwd
            .ancestors()
            .find(|dir| dir.join("course/info.toml").is_file())
            .ok_or("no course found here (expected course/info.toml in this directory or a parent)")?
            .to_path_buf();
        let text = fs::read_to_string(root.join("course/info.toml"))?;
        let info: Info = toml::from_str(&text).map_err(|e| format!("course/info.toml: {e}"))?;
        if info.exercises.is_empty() {
            return Err("course/info.toml lists no exercises".into());
        }
        Ok(Course { root, exercises: info.exercises })
    }

    /// The untouched original of an exercise.
    pub fn pristine(&self, ex: &Exercise) -> PathBuf {
        self.root.join("course/exercises").join(ex.file())
    }

    pub fn solution(&self, ex: &Exercise) -> PathBuf {
        self.root.join("course/solutions").join(ex.file())
    }

    /// The learner's own copy, the one being watched.
    pub fn working(&self, ex: &Exercise) -> PathBuf {
        self.root.join("exercises").join(ex.file())
    }

    pub fn bin_dir(&self) -> PathBuf {
        self.root.join(".ccourse/bin")
    }

    /// Shortens a path for display and for compiler messages.
    pub fn relative<'a>(&self, path: &'a Path) -> &'a Path {
        path.strip_prefix(&self.root).unwrap_or(path)
    }

    /// Gives the learner a copy of every exercise they do not have yet.
    pub fn ensure_working(&self) -> Result<()> {
        for ex in &self.exercises {
            if !self.working(ex).exists() {
                self.reset(ex)?;
            }
        }
        Ok(())
    }

    /// Replaces the learner's copy with the original.
    pub fn reset(&self, ex: &Exercise) -> Result<()> {
        let to = self.working(ex);
        if let Some(parent) = to.parent() {
            fs::create_dir_all(parent)?;
        }
        fs::copy(self.pristine(ex), &to)
            .map_err(|e| format!("cannot copy {}: {e}", self.pristine(ex).display()))?;
        Ok(())
    }

    pub fn index_of(&self, name: &str) -> Option<usize> {
        self.exercises.iter().position(|ex| ex.name == name)
    }
}

#[derive(Serialize, Deserialize, Default)]
pub struct State {
    #[serde(default)]
    pub current: String,
    #[serde(default)]
    pub done: Vec<String>,
}

impl State {
    fn path(course: &Course) -> PathBuf {
        course.root.join(".ccourse/state.toml")
    }

    pub fn load(course: &Course) -> State {
        fs::read_to_string(State::path(course))
            .ok()
            .and_then(|text| toml::from_str(&text).ok())
            .unwrap_or_default()
    }

    pub fn save(&self, course: &Course) -> Result<()> {
        let path = State::path(course);
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent)?;
        }
        fs::write(path, toml::to_string(self)?)?;
        Ok(())
    }

    pub fn is_done(&self, ex: &Exercise) -> bool {
        self.done.contains(&ex.name)
    }

    pub fn mark_done(&mut self, ex: &Exercise) {
        if !self.is_done(ex) {
            self.done.push(ex.name.clone());
        }
    }

    /// The first unfinished exercise after `from`, wrapping round to the start.
    pub fn next_pending(&self, course: &Course, from: usize) -> Option<usize> {
        let n = course.exercises.len();
        (1..=n)
            .map(|step| (from + step) % n)
            .find(|&i| !self.is_done(&course.exercises[i]))
    }
}
