//! Watch mode: re-check the current exercise whenever its file is saved.

use std::fs;
use std::io::{self, IsTerminal, Write};
use std::path::Path;
use std::thread;
use std::time::{Duration, SystemTime};

use crossterm::event::{self, Event, KeyCode, KeyEvent, KeyEventKind, KeyModifiers};
use crossterm::terminal;

use crate::Result;
use crate::check::{self, BOLD, CYAN, DIM, GREEN, Outcome, RED, RESET, YELLOW};
use crate::course::{Course, State};

const POLL: Duration = Duration::from_millis(200);
const MAX_DETAIL_LINES: usize = 60;

/// Restores the terminal however watch mode ends.
struct RawMode;

impl RawMode {
    fn enter() -> Result<RawMode> {
        terminal::enable_raw_mode()?;
        print!("\x1b[?25l");
        Ok(RawMode)
    }
}

impl Drop for RawMode {
    fn drop(&mut self) {
        let _ = terminal::disable_raw_mode();
        print!("\x1b[?25h");
        let _ = io::stdout().flush();
    }
}

struct Session<'a> {
    course: &'a Course,
    state: State,
    index: usize,
    outcome: Outcome,
    hints_shown: usize,
    note: Option<String>,
}

pub fn run(course: &Course) -> Result<()> {
    if !io::stdin().is_terminal() || !io::stdout().is_terminal() {
        return Err("watch mode needs an interactive terminal".into());
    }
    course.ensure_working()?;
    let state = State::load(course);
    let index = course
        .index_of(&state.current)
        .or_else(|| course.exercises.iter().position(|ex| !state.is_done(ex)))
        .unwrap_or(0);

    let _raw = RawMode::enter()?;
    let mut session = Session {
        course,
        state,
        index,
        outcome: Outcome::Passed(String::new()),
        hints_shown: 0,
        note: None,
    };
    session.go_to(index)?;
    session.watch()?;
    show("");
    Ok(())
}

impl Session<'_> {
    fn watch(&mut self) -> Result<()> {
        let mut seen = modified(&self.file());
        loop {
            if event::poll(POLL)? {
                match event::read()? {
                    Event::Key(key) if key.kind == KeyEventKind::Press => {
                        if is_quit(&key) {
                            return Ok(());
                        }
                        self.note = None;
                        match key.code {
                            KeyCode::Char('h') => self.hint(),
                            KeyCode::Char('n') => self.next()?,
                            KeyCode::Char('r') => self.recheck()?,
                            KeyCode::Char('x') => self.reset()?,
                            KeyCode::Char('l') => {
                                if let Some(index) = self.list()? {
                                    self.go_to(index)?;
                                }
                            }
                            _ => {}
                        }
                        self.render();
                        seen = modified(&self.file());
                    }
                    Event::Resize(..) => self.render(),
                    _ => {}
                }
            }
            let now = modified(&self.file());
            if now.is_some() && now != seen {
                // Give the editor a moment to finish writing.
                thread::sleep(Duration::from_millis(50));
                seen = modified(&self.file());
                self.note = None;
                self.recheck()?;
                self.render();
            }
        }
    }

    fn file(&self) -> std::path::PathBuf {
        self.course.working(&self.course.exercises[self.index])
    }

    fn go_to(&mut self, index: usize) -> Result<()> {
        self.index = index;
        self.hints_shown = 0;
        self.state.current = self.course.exercises[index].name.clone();
        self.recheck()?;
        self.render();
        Ok(())
    }

    fn recheck(&mut self) -> Result<()> {
        let ex = &self.course.exercises[self.index];
        show(&format!("{DIM}Checking {}…{RESET}\n", ex.name));
        self.outcome = check::check(self.course, ex, &self.course.working(ex));
        if matches!(self.outcome, Outcome::Passed(_)) {
            self.state.mark_done(ex);
        }
        self.state.save(self.course)
    }

    fn hint(&mut self) {
        let total = self.course.exercises[self.index].hints.len();
        if total == 0 {
            self.note = Some("This exercise has no hints.".to_string());
        } else if self.hints_shown == total {
            self.note = Some("That was the last hint.".to_string());
        } else {
            self.hints_shown += 1;
        }
    }

    fn next(&mut self) -> Result<()> {
        if !matches!(self.outcome, Outcome::Passed(_)) {
            self.note = Some("This exercise is not passing yet. Press l to jump to another one.".to_string());
            return Ok(());
        }
        match self.state.next_pending(self.course, self.index) {
            Some(index) => self.go_to(index),
            None => {
                self.note = Some("You have finished every exercise in the course. Well done!".to_string());
                Ok(())
            }
        }
    }

    fn reset(&mut self) -> Result<()> {
        self.note = Some("Reset this file to its original state? Your changes will be lost. Press y to confirm.".to_string());
        self.render();
        self.note = None;
        if read_key()?.code == KeyCode::Char('y') {
            self.course.reset(&self.course.exercises[self.index])?;
            self.hints_shown = 0;
            self.recheck()?;
            self.note = Some("File reset. Reload it in your editor.".to_string());
        }
        Ok(())
    }

    fn render(&self) {
        let ex = &self.course.exercises[self.index];
        let total = self.course.exercises.len();
        let done = self.course.exercises.iter().filter(|ex| self.state.is_done(ex)).count();
        let filled = done * 30 / total;

        let mut s = format!(
            "{BOLD}{}{RESET} {DIM}·{RESET} {} {DIM}·{RESET} exercise {} of {total} {DIM}·{RESET} {}\n",
            ex.name,
            ex.dir,
            self.index + 1,
            ex.kind.label(),
        );
        s += &format!(
            "{GREEN}{}{RESET}{DIM}{}{RESET} {done}/{total} done\n\n",
            "█".repeat(filled),
            "░".repeat(30 - filled),
        );
        s += &format!("Open in your editor: {CYAN}{}{RESET}\n\n", self.course.relative(&self.file()).display());

        match &self.outcome {
            Outcome::Passed(output) => {
                s += &format!("{GREEN}{BOLD}✓ Passed{RESET}\n\n");
                if !output.trim().is_empty() {
                    s += &format!("Output:\n{}\n", clip(output));
                }
                s += &format!(
                    "Reference solution: {CYAN}{}{RESET}\n",
                    self.course.relative(&self.course.solution(ex)).display()
                );
            }
            Outcome::Failed { title, detail } => {
                s += &format!("{RED}{BOLD}✗ {title}{RESET}\n\n{}", clip(detail));
            }
        }

        for (i, hint) in ex.hints.iter().take(self.hints_shown).enumerate() {
            s += &format!("\n{YELLOW}Hint {} of {}:{RESET} {hint}\n", i + 1, ex.hints.len());
        }
        if let Some(note) = &self.note {
            s += &format!("\n{YELLOW}{note}{RESET}\n");
        }

        let key = |k: &str, what: &str| format!("{BOLD}{k}{RESET} {what}");
        let mut keys = Vec::new();
        if matches!(self.outcome, Outcome::Passed(_)) {
            keys.push(key("n", "next"));
        }
        keys.extend([key("h", "hint"), key("l", "list"), key("r", "re-run"), key("x", "reset"), key("q", "quit")]);
        s += &format!("\n{}\n", keys.join(&format!(" {DIM}·{RESET} ")));
        show(&s);
    }

    /// The exercise list. Returns the exercise to jump to, if one was chosen.
    fn list(&self) -> Result<Option<usize>> {
        let total = self.course.exercises.len();
        let mut cursor = self.index;
        loop {
            let rows = terminal::size().map_or(24, |(_, rows)| rows as usize).saturating_sub(4).max(3);
            let top = cursor.saturating_sub(rows / 2).min(total.saturating_sub(rows));

            let mut s = format!("{BOLD}Exercises{RESET}\n\n");
            for (i, ex) in self.course.exercises.iter().enumerate().skip(top).take(rows) {
                let mark = if self.state.is_done(ex) { format!("{GREEN}✓{RESET}") } else { format!("{DIM}·{RESET}") };
                let pointer = if i == cursor { format!("{CYAN}>{RESET}") } else { " ".to_string() };
                let name = if i == cursor { format!("{BOLD}{}{RESET}", ex.name) } else { ex.name.clone() };
                s += &format!("{pointer} {mark} {:>3}  {DIM}{:<16}{RESET} {name} {DIM}({}){RESET}\n", i + 1, ex.dir, ex.kind.label());
            }
            s += &format!("\n{BOLD}↑ ↓{RESET} move {DIM}·{RESET} {BOLD}enter{RESET} open {DIM}·{RESET} {BOLD}q{RESET} back\n");
            show(&s);

            let key = read_key()?;
            match key.code {
                KeyCode::Up | KeyCode::Char('k') => cursor = cursor.saturating_sub(1),
                KeyCode::Down | KeyCode::Char('j') => cursor = (cursor + 1).min(total - 1),
                KeyCode::Enter => return Ok(Some(cursor)),
                KeyCode::Esc | KeyCode::Char('q') | KeyCode::Char('l') => return Ok(None),
                _ if is_quit(&key) => return Ok(None),
                _ => {}
            }
        }
    }
}

fn is_quit(key: &KeyEvent) -> bool {
    key.code == KeyCode::Char('q')
        || (key.code == KeyCode::Char('c') && key.modifiers.contains(KeyModifiers::CONTROL))
}

/// Waits for the next key press.
fn read_key() -> Result<KeyEvent> {
    loop {
        if let Event::Key(key) = event::read()?
            && key.kind == KeyEventKind::Press
        {
            return Ok(key);
        }
    }
}

fn modified(path: &Path) -> Option<SystemTime> {
    fs::metadata(path).and_then(|meta| meta.modified()).ok()
}

/// Keeps a long compiler or sanitizer report from flooding the screen.
fn clip(text: &str) -> String {
    let lines: Vec<&str> = text.lines().collect();
    if lines.len() <= MAX_DETAIL_LINES {
        return lines.iter().map(|line| format!("{line}\n")).collect();
    }
    let mut clipped: String = lines[..MAX_DETAIL_LINES].iter().map(|line| format!("{line}\n")).collect();
    clipped += &format!("{RESET}{DIM}… {} more lines not shown{RESET}\n", lines.len() - MAX_DETAIL_LINES);
    clipped
}

/// Replaces the screen contents. Raw mode needs explicit carriage returns.
fn show(text: &str) {
    let mut out = io::stdout().lock();
    let _ = write!(out, "\x1b[2J\x1b[3J\x1b[H{}", text.replace('\n', "\r\n"));
    let _ = out.flush();
}
