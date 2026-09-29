this file was made using ai but checked and edited by Lucas Zahlan

## Git and GitHub Workflow

Git is used so multiple team members can work on the firmware without overwriting each other's work. Each person should do their development on their own branch, then merge completed work into the stable `master` branch.

> **PLEASE ALWAYS DEVELOP USING BRANCHES. DO NOT DEVELOP DIRECTLY ON `master`.**

Some GitHub repositories use `main` instead of `master` as their default branch. This repository uses `master`. If your local repository displays `main`, use `main` anywhere this guide says `master`.

### Using Codex or Other IDE Git Agents

Codex and similar IDE-integrated agents are generally good at performing routine Git operations such as:

- creating and switching branches
- checking Git status
- staging files
- committing changes
- pulling changes
- rebasing
- pushing branches

You can ask the agent to perform these operations, but you should still understand the commands below to a basic degree so that you know what it is doing.

**Always verify which branch you are on before allowing an agent to merge, rebase, or force-push anything.**

---

## 1. Check Your Current Branch and Repository Status

Before starting work, check:

```bash
git status
```

This tells you:

- your current branch
- which files have changed
- which files are staged
- which files are untracked

You can also list your branches with:

```bash
git branch
```

The active branch has a `*` beside it.

Example:

```text
  master
* lz/ultrasonic
```

If you see:

```text
* master
```

and you are about to start modifying firmware, **create or switch to a development branch first.**

---

## 2. ALWAYS Develop on a Branch

Before beginning new work, update `master`:

```bash
git switch master
git pull
```

Then create your development branch:

```bash
git switch -c initials/feature-name
```

Branch names should follow:

```text
initials/feature-name
```

For example:

```bash
git switch -c lz/ultrasonic
```

Other examples:

```text
lz/motor-control
ab/imu
jd/steering
lz/adc-fix
ab/pwm-timing
```

Use your own initials at the beginning so everyone can immediately see who owns a branch.

### Do NOT do this:

```bash
git switch master

# Start developing directly here
```

Instead:

```bash
git switch master
git pull
git switch -c lz/my-feature
```

Then begin making changes.

---

## 3. Pull

`git pull` retrieves the latest changes from GitHub and updates your local branch.

The most common use is updating `master`:

```bash
git switch master
git pull
```

Do this before creating a new branch so that your new work starts from the latest version of the project.

Typical start of new work:

```bash
git switch master
git pull
git switch -c lz/my-feature
```

If your development branch already exists, switch back to it afterward:

```bash
git switch lz/my-feature
```

---

## 4. Stage Changes

Git does not automatically include every modified file in a commit.

First check what changed:

```bash
git status
```

Stage a specific file:

```bash
git add src/main.c
```

Stage several specific files:

```bash
git add src/motor.c include/motor.h
```

Or stage all current changes:

```bash
git add .
```

Check again:

```bash
git status
```

Files shown under:

```text
Changes to be committed:
```

will be included in your next commit.

---

## 5. Commit Changes

A commit saves a checkpoint of the files that you staged.

Example:

```bash
git commit -m "Add ultrasonic distance reading"
```

Use short but descriptive commit messages.

Good examples:

```text
Add motor PWM control
Implement ADC initialization
Fix steering servo limits
Add ultrasonic sensor interface
Fix UART baud configuration
```

Avoid vague messages such as:

```text
stuff
changes
update
working
asdf
```

It is completely normal to make multiple commits while working on one branch.

For example:

```text
Add ultrasonic initialization
Add ultrasonic distance calculation
Fix ultrasonic timeout handling
```

---

## 6. Push Your Branch

Pushing uploads your local commits to GitHub.

The first time you push a newly created branch:

```bash
git push -u origin lz/my-feature
```

For example:

```bash
git push -u origin lz/ultrasonic
```

After that, you can normally use:

```bash
git push
```

Pushing your development branch does **not** modify `master`.

---

## 7. Rebase and Merge Conflicts

While you are working on your branch, another team member may finish their work and merge it into `master`.

You may then have:

```text
master:
someone else's new changes

your branch:
your changes based on an older version of master
```

Rebasing updates your branch so that your work is placed on top of the newest version of `master`.

This becomes especially important if **you and another person changed the same file or the same part of a file**.

If the changes do not interfere with each other, Git can usually rebase automatically.

If the changes overlap, Git may stop and give you a **merge conflict**. At that point, you are responsible for deciding which code should remain, whether one version should replace the other, or whether both changes should be combined.

Before merging your branch, update `master`:

```bash
git switch master
git pull
```

Return to your branch:

```bash
git switch lz/my-feature
```

Then rebase onto the updated `master`:

```bash
git rebase master
```

Conceptually, suppose the project starts like this:

```text
A---B master
     \
      C---D lz/my-feature
```

Someone else then adds changes to `master`:

```text
A---B---E---F master
     \
      C---D lz/my-feature
```

After:

```bash
git rebase master
```

your branch becomes:

```text
A---B---E---F master
             \
              C'---D' lz/my-feature
```

Your work is now based on the newest version of `master`.

---

## 8. Resolving a Rebase Conflict

If Git stops during a rebase because both versions changed the same area, run:

```bash
git status
```

Git will identify the conflicting files.

Inside a conflicted file, you may see something similar to:

```text
<<<<<<< HEAD
code currently in master
=======
code from your branch
>>>>>>> lz/my-feature
```

You must edit this section manually.

Decide whether to:

- keep the code from `master`
- keep your branch's code
- combine both versions

Then completely remove the Git conflict markers:

```text
<<<<<<<
=======
>>>>>>>
```

Once the file contains the correct final code, stage it:

```bash
git add path/to/file
```

Then continue the rebase:

```bash
git rebase --continue
```

If another conflict appears, repeat the process.

If something goes badly and you want to return to the state before the rebase:

```bash
git rebase --abort
```

---

## 9. Pushing After a Rebase

Rebasing changes the commit history of your development branch.

Because of this, Git may reject a normal:

```bash
git push
```

after you have rebased a branch that was already pushed to GitHub.

If Git requires a force push, **first check your branch**:

```bash
git branch
```

Make absolutely sure you are on your own development branch:

```text
* lz/my-feature
  master
```

and **NOT**:

```text
* master
```

A force push rewrites the remote commit history of the branch being pushed.

### NEVER FORCE-PUSH `master`.

Force pushes should only be used on your own development branch when necessary after operations such as rebasing.

---

## 10. Switching and Viewing Branches

See your local branches:

```bash
git branch
```

See both local and remote branches:

```bash
git branch -a
```

Create a new branch:

```bash
git switch -c lz/feature-name
```

Switch to an existing branch:

```bash
git switch lz/feature-name
```

Return to `master`:

```bash
git switch master
```

---

## 11. Typical Workflow for New Work

Before starting a new feature:

```bash
git switch master
git pull
git switch -c lz/my-feature
```

Develop your feature.

When you reach a useful checkpoint:

```bash
git status
git add .
git commit -m "Describe the change"
```

Push it to GitHub:

```bash
git push -u origin lz/my-feature
```

Continue developing and make additional commits as necessary:

```bash
git add .
git commit -m "Describe the next change"
git push
```

---

## 12. Updating an Existing Development Branch

If you come back later and other work has since been merged into `master`:

```bash
git switch master
git pull
```

Then:

```bash
git switch lz/my-feature
git rebase master
```

Resolve any conflicts if Git asks you to.

Your branch is now based on the newest version of `master`.

---

## 13. Example Feature Workflow

Suppose Lucas is responsible for implementing the ultrasonic sensor.

Start from the latest `master`:

```bash
git switch master
git pull
```

Create the branch:

```bash
git switch -c lz/ultrasonic
```

Develop the ultrasonic functionality.

Check the changes:

```bash
git status
```

Stage them:

```bash
git add src/ultrasonic.c include/ultrasonic.h
```

Commit:

```bash
git commit -m "Implement ultrasonic sensor interface"
```

Push:

```bash
git push -u origin lz/ultrasonic
```

If other code is merged into `master` before the ultrasonic work is finished:

```bash
git switch master
git pull

git switch lz/ultrasonic
git rebase master
```

Resolve any conflicts, then push the updated branch.

Once the feature satisfies the requirements below, it can be merged into `master`.

---

## 14. Requirements Before Merging to `master`

`master` should represent the stable integrated version of the hovercraft firmware.

Before merging a development branch into `master`, make sure:

- the feature is reasonably complete
- the code compiles
- obvious warnings or errors have been addressed
- the code does not break existing functionality
- any hardware-specific changes have been checked where practical
- conflicts with newer changes in `master` have been resolved
- the branch has been updated/rebased against the latest `master`
- commit messages reasonably explain what was changed
- another team member can understand the purpose of the changes
- experimental or temporary debugging code is removed unless intentionally required

Prefer merging changes through a GitHub Pull Request so that the team can see what is being added before it enters `master`.

### Never:

```text
Develop directly on master
Force-push master
Merge known broken code into master
Overwrite someone else's work without resolving the conflict
```

---

## Quick Reference

```bash
# Check status
git status

# See branches
git branch

# Update master
git switch master
git pull

# Create development branch
git switch -c lz/my-feature

# Switch branch
git switch lz/my-feature

# Stage everything
git add .

# Commit
git commit -m "Describe the change"

# First push of branch
git push -u origin lz/my-feature

# Later pushes
git push

# Update your branch from master
git switch master
git pull
git switch lz/my-feature
git rebase master

# Abort a problematic rebase
git rebase --abort
```

## Most Important Rule

**PLEASE ALWAYS DEVELOP USING BRANCHES.**

If you are about to change firmware and your terminal shows:

```text
(master)
```

create or switch to your own branch first:

```bash
git switch -c initials/feature-name
```