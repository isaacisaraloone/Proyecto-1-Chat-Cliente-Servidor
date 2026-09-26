using System;
using System.Collections.Generic;
using System.Security.Cryptography.X509Certificates;
using System.Text.Json;

namespace Cliente
{
	public class AnalizarComando
	{
		public List<string> ProcesarEntradaUsuario(string input)
		{
			var mensajesJSON = new List<string>();
			string[] partes = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);

			if (partes.Length == 0) return mensajesJSON;

			string comando = partes[0].ToLower();

			try
			{
				switch (comando)
				{
					case "/estado":
						if (partes.Length < 2)
						{
							Console.WriteLine("Uso: /estado <AWAY|BUSY|ACTIVE");
							break;
						}
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "STATUS", estado = partes[1].ToUpper() }));
						break;

					case "/usuarios":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "USERS" }));
						break;

					case "/privado":
						if (partes.Length < 3)
						{
							Console.WriteLine("Uso:");
						}
						string mensajeTexto = string.Join(" ", partes.Length - 2);
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;

					case "/publico":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;

					case "/nuevasala":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;

					case "/invitar":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;

					case "/salausuarios":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;

					case "/mensajesala":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;

					case "/irse":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;

					case "/cerrar":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = ""}));
						break;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error al procesar el comando: {ex.Message}");
			}

			return mensajesJSON;
		}

		public void ProcesarMensajeServidor(string json)
		{
		}

		public void AyudaVisual()
		{
			Console.WriteLine("--- Comandos del Cliente ---");
			Console.WriteLine("/estado <AWAY|BUSY|ACTIVE>	- Cambia tu estado");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
			Console.WriteLine("");
		}

		public string GenerarIdentificacion(string username)
		{
			return JsonSerializer.Serialize(new { type = "IDENTIFY", username = username});
		}
	}
}