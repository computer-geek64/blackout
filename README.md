# Blackout

### Automated Git History Rewriter for Sensitive Information

## Description

**Blackout** is a Git VCS tool for rewriting commit history to hide sensitive information. It was built with [libgit2](https://libgit2.org/), a low-level C implementation of Git core methods, far beneath even the foundational Git plumbing. This is advantageous because it helps to maximize time and space efficiency, as parsing through blob trees for each node in the commit DAG (Directed Acyclic Graph) is very expensive, especially for large repositories.

## Installation

Build **Blackout** with Make:
```bash
make clean
make
```

Install/Uninstall:
```bash
sudo make install
sudo make uninstall
```

## Usage

### Censor all instances of a string

This is the core functionality of **Blackout**. Pass the target and replacement strings as CLI arguments, and it replaces all instances of the string in the entire Git commit history:

```bash
blackout 'apikey123' '******'
```

## Developers

Ashish D'Souza - [@computer-geek64](https://github.com/computer-geek64)

## Releases

The current stable release for **Blackout** is [v1.0.0](https://github.com/computer-geek64/blackout/releases/latest)

## Versioning

This project is uses the [Git](https://git-scm.com/) Version Control System (VCS).

## License

This project is licensed under the [MIT License](LICENSE).
