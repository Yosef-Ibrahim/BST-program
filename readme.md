# 🌳 AVL Tree Visualizer (BST with Auto-Balancing)

A professional, interactive educational tool built with **C++** and **Raylib** to visualize and interact with Binary Search Trees featuring real-time **AVL balancing**. Watch your data structures come to life with dynamic animations and extensive traversal options.

> This project was developed as a comprehensive guide and tool for understanding self-balancing trees.

---

## 🌟 Visual Showcase

### Primary Interface & Auto-Balancing
Here is the main interface of the AVL Tree Visualizer, showing a complex, color-coded AVL tree after multiple insertions. You can see how the tree maintains its balance automatically.

![AVL Tree Main Interface](Program.png)

---

## 🔥 Professional Features (All Included)

This visualizer is packed with features designed for both learning and practical testing.

### 🧠 Core Tree operations
* **AVL Insertion:** Insert nodes and watch the AVL algorithm perform necessary Right, Left, Left-Right, or Right-Left rotations instantly. A popup message confirms the rotation type.
* **Delete Node:** Delete a specific value and see the tree re-structure and re-balance itself.
* **Clear ALL Tree:** Wipes the entire tree structure to start over instantly with a single click.

### 📊 Live Tree Statistics (Real-Time)
A dedicated panel in the top-right corner displays live tree metrics:
* **Total Nodes:** Real-time count of all nodes currently in the tree.
* **Tree Height:** Live calculation of the tree's height, proving the efficiency of AVL balancing.

### 🔍 Search, Successor & Predecessor
* **Find Predecessor:** Calculates and displays the in-order predecessor of any input value.
* **Find Successor:** Calculates and displays the in-order successor of any input value.
* **Path Highlighting:** When finding predecessors or successors, the tool **glows the entire search path in yellow**, visually teaching you how the tree is traversed.

### 🖨️ Complete Traversal Printing (4 Methods)
Display the contents of your tree in any of the four standard traversal methods. The results are formatted as a vector (e.g., `[ 15, 30, 45 ]`) and displayed directly at the top of the workspace.

| Method | Description |
| :--- | :--- |
| **Print Pre-Order** | Roots first, then left and right subtrees. |
| **Print In-Order** | Prints data in sorted order. |
| **Print Post-Order** | Nodes processed after their descendants. |
| **Print Breadth-First** | Processes nodes level by level. |

### 🎨 Modern Visual Experience
* **Level-Based Coloring:** Each tree level is automatically assigned a distinct color for immediate structural recognition.
* **Camera System (Zoom & Pan):**
    * **Mouse Wheel:** Smoothly zoom in and out to handle huge trees.
    * **Right Click & Drag:** Pan across the canvas to explore large structures.
* **Dynamic Node Spacing:** Nodes automatically adjust their distance based on screen size, ensuring a clear view.
* **Professional UI:** A modern sidebar, clean buttons, and non-intrusive popup notifications.

---

## 🎮 How To Use

1.  **Run** the application.
2.  Use the **Control Panel** on the left.
3.  **Enter a value** in the Value box.
4.  **Click an action** (Insert, Delete, or any Find operation).
5.  **View Results:** Tree updates are animated, traversals are printed at the top, and rotations/find results appear in a popup and the status box below.

---

## 📄 License
This project is open-source and available under the [MIT License](LICENSE).