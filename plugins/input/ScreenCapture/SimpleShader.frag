#version 120

uniform sampler2D texture;
uniform vec4 sourceRect;

varying vec2 vTexCoord;

void main()
{
    vec2 uv = sourceRect.xy + vTexCoord * sourceRect.zw;
    gl_FragColor = texture2D(texture, uv);
    //vec4 c = texture2D(texture, uv);
    //gl_FragColor = vec4(c.r, c.g, c.b, c.a);
}
