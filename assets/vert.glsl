#version 120
uniform vec2 iResolution;
void main() {
  vec2 pos = (gl_Vertex.xy / iResolution) * 2.0 - vec2(1, 1);
  gl_Position = vec4(pos, 0.0, 1.0);
}
