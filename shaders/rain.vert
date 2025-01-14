#version 330 core

// Layout-ul de intrare: poziția fiecărui vertex
layout(location = 0) in vec3 position;

// Uniforme pentru transformări
uniform mat4 model;      // Matricea de transformare model
uniform mat4 view;       // Matricea de transformare view
uniform mat4 projection; // Matricea de proiecție

// Funcția principală
void main() {
    // Calcularea poziției finale a vertexului
    gl_Position = projection * view * model * vec4(position, 1.0);
}
