// strings2.rs
//
// Make me compile without changing the function signature!
//
// DONE

fn main() {
    let word = String::from("green"); // Try not changing this line :)
    if is_a_color_word(&word) {
        println!("That is a color word I know!");
    } else {
        println!("That is not a color word I know.");
    }
}

fn is_a_color_word(attempt: &str) -> bool {
    // *attempt == String::from("green") || *attempt == String::from("blue") || *attempt == String::from("red")
    attempt == "green" || attempt == "blue" || attempt == "red"
}
