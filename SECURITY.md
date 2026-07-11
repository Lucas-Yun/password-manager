# Security Policy

## Supported Versions
We only provide security patches for the following versions of password-manager.
Versions not marked with ✅ will not receive any security fixes.

| Version | Supported          |
| ------- | ------------------ |
| 5.1.x   | :white_check_mark: |
| 4.0.x   | :white_check_mark: |
| 5.0.x   | :x:                |
| < 4.0   | :x:                |

## Reporting a Vulnerability
This is a password management tool, which involves sensitive user credential data.
**Please DO NOT create public GitHub Issues to report security vulnerabilities**, as public disclosure will expose risks to all users.

### How to submit vulnerabilities
1. Send vulnerability details to our dedicated security email: `yunbosheng2022@163.com`
2. Your email should include:
   - Brief description of the vulnerability type (e.g. plaintext storage, encryption flaw, password leakage, input injection)
   - Reproduction steps to trigger the flaw
   - Potential impact severity (low / medium / high / critical)
   - Suggested fix (if you have a solution)
   - Your contact information for follow-up communication

### Response Process
1. We will acknowledge receipt of your vulnerability report within **3 working days**.
2. We will evaluate the severity and confirm whether we will fix this vulnerability within 7 days.
    - If the vulnerability is accepted: We will develop a patch, release a new secure version, and credit you in the release notes (if you agree). We ask for a 90-day private disclosure window before publicizing the flaw.
    - If we decline to fix: We will send a detailed explanation of our reasoning via email.
3. Critical vulnerabilities affecting user password safety will be prioritized and patched as soon as possible.

## Security Best Practices for Users
1. Always use the latest supported version of password-manager. Outdated versions lack security protection.
2. Do not store the program database on public cloud disks without extra encryption.
3. Use a complex master password to lock your local password vault.
4. Back up your encrypted vault file regularly to avoid data loss.
