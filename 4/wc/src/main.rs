use std::io::BufRead;

fn main() {
    let mut reader = std::io::BufReader::new(std::io::stdin());
    let lines: usize = reader.lines().filter_map(Result::ok).map(|s| s.split_whitespace().count()).sum();
    println!("{}", lines);
}
