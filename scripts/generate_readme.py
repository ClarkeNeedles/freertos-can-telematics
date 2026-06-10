import os
from google import genai
from google.genai import errors
from dotenv import load_dotenv

def generate_readme():
    # Get the absolute path of the root/scripts/ folder
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Secure absolute paths relative to the script location
    log_dir = os.path.abspath(os.path.join(script_dir, "../logbook"))
    output_path = os.path.abspath(os.path.join(script_dir, "../README.md"))
    env_path = os.path.abspath(os.path.join(script_dir, "../.env"))

    # Load .env vars explicitly from the root directory path
    load_dotenv(dotenv_path=env_path)
    
    # Verify API key
    if not os.environ.get("GEMINI_API_KEY"):
        print("GEMINI_API_KEY not found.")
        print(f"Please create a '.env' file in your root folder: {os.path.dirname(script_dir)}")
        print("See '.env.example' for reference.")
        return

    # Initialize Gemini client
    try:
        client = genai.Client()
    except Exception as e:
        print(f"Initialization Error: {e}")
        return
    
    if not os.path.exists(log_dir):
        print(f"Error: Could not find the logbook folder at: {log_dir}")
        return

    # Gather and merge all logbook markdown files chronologically
    log_content = ""
    try:
        log_files = sorted([f for f in os.listdir(log_dir) if f.endswith(".md")])
    except Exception as e:
        print(f"Error reading logbook directory: {e}")
        return
    
    if not log_files:
        print(f"Error: No markdown (.md) logs found inside the logbook folder at: {log_dir}")
        return

    for filename in log_files:
        file_path = os.path.join(log_dir, filename)
        with open(file_path, "r", encoding="utf-8") as f:
            log_content += f"\n\n--- Start of Log: {filename} ---\n"
            log_content += f.read()
            log_content += f"\n--- End of Log: {filename} ---\n"

    # Craft the instructions for the free Gemini 2.5 Flash model
    prompt = (
        f"You are an expert technical writer. Read through these project logbook entries "
        f"and synthesize them into a clean, professional, and comprehensive README.md file.\n\n"
        f"Your instructions:\n"
        f"- Analyze the free-form notes across all logs to extract the actual features implemented.\n"
        f"- Look at the metadata at the top of the most recent log file to determine the latest project status and version.\n"
        f"- Generate a structured README containing: Project Name, a high-level overview, a logically categorized list of all completed features, the tech stack (infer this from their notes), and current version/status.\n"
        f"- Ignore conversational developer ramblings, temporary bugs, or unfinished thoughts. Focus strictly on working capabilities.\n"
        f"- Output ONLY raw, valid markdown syntax. Do not wrap your entire response inside standard markdown code blocks (e.g., do not start with ```markdown).\n\n"
        f"Project Logs Data:\n{log_content}"
    )

    print(f"Found {len(log_files)} log file(s). Processing with Gemini AI...")

    # Call the free Gemini Flash model
    try:
        response = client.models.generate_content(
            model='gemini-2.5-flash',
            contents=prompt,
        )
        
        # Save the generated text directly to your root README.md
        with open(output_path, "w", encoding="utf-8") as f:
            f.write(response.text)
            
        print(f"Success! README.md has been generated and updated at: {output_path}")
        
    except errors.APIError as e:
        print(f"Gemini API Error: {e}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    generate_readme()
